"""通过真实C++主控、AIClient和SSE验证执行器，固定回复不代表教学质量。"""
import json
from pathlib import Path
import subprocess
import sys
import tempfile
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from threading import Thread

sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'scripts'))
import tutor_experiment as experiment


class NativeFixture(BaseHTTPRequestHandler):
    calls=[]
    replies=[]
    behavior='normal'

    def log_message(self,*_):pass

    def do_POST(self):
        request=json.loads(self.rfile.read(int(self.headers['Content-Length'])))
        type(self).calls.append(request)
        if self.behavior=='compatibility' and 'stream_options' in request:
            self.send_response(400);self.end_headers()
            self.wfile.write(b'{"error":{"message":"unsupported stream_options include_usage"}}');return
        if self.behavior=='redirect':
            self.send_response(307);self.send_header('Location','https://example.invalid/never');self.end_headers();return
        context=json.loads(request['messages'][1]['content'])
        actions=[];state='waiting_student'
        # 固定分类只检验正式输入的工具协议，不生成知识诊断或质量成绩。
        results=[]
        for message in request['messages'][2:]:
            if message['role']=='user':
                try:results.extend(json.loads(message['content']).get('toolResults',[]))
                except ValueError:pass
        if context['event']['type']=='question_answer' and not results:
            actual=context['currentInputs'][0]
            actions=[{'id':'fixture-classify','tool':'classify_input','args':{'interactionId':actual['interactionId'],
                      'isAnswer':False,'reason':'本机工程夹具仅确认绑定，不对知识或作答评分。'}}]
            state='continue'
        value=json.dumps({'teaching':{'helpType':'explanation'},'message':'本机协议夹具，仅确认输入已绑定；不评价知识或诊断质量。',
                          'actions':actions,'state':state},ensure_ascii=False)
        self.replies.append(value)
        self.send_response(200);self.send_header('Content-Type','text/event-stream');self.end_headers()
        try:
            if self.behavior=='timeout':
                import time
                chunk={'model':'isolated-tutor-fixture','choices':[{'delta':{'content':value[:20]},'finish_reason':None}]}
                self.wfile.write(('data: '+json.dumps(chunk)+'\n\n').encode());self.wfile.flush()
                time.sleep(2);return
            for index in range(0,len(value),17):
                chunk={'model':'isolated-tutor-fixture','choices':[{'delta':{'content':value[index:index+17]},'finish_reason':None}]}
                self.wfile.write(('data: '+json.dumps(chunk,ensure_ascii=False)+'\n\n').encode());self.wfile.flush()
            usage=None if self.behavior=='unknown' else {'prompt_tokens':5,'completion_tokens':4,'total_tokens':9}
            final={'model':'isolated-tutor-fixture','choices':[{'delta':{},'finish_reason':'stop'}],'usage':usage}
            self.wfile.write(('data: '+json.dumps(final)+'\n\ndata: [DONE]\n\n').encode())
        except (ConnectionError,OSError):pass


def main(driver,output=None):
    from contextlib import nullcontext
    context=nullcontext(str(output)) if output else tempfile.TemporaryDirectory(prefix='tutor-native-')
    with context as directory:
        root=Path(directory);root.mkdir(parents=True,exist_ok=True)
        for endpoint,expected in [('https://example.invalid/v1','https://example.invalid/v1/chat/completions'),
                                  ('https://example.invalid/v1///','https://example.invalid/v1/chat/completions'),
                                  ('https://example.invalid/v1/chat/completions///','https://example.invalid/v1/chat/completions')]:
            process=subprocess.run([str(driver),'--normalize-evaluation-endpoint',endpoint],capture_output=True,timeout=10)
            assert process.returncode==0 and process.stdout.decode()==expected
        dataset=experiment.ROOT/'tests/fixtures/ai-tutor/candidate-v2.json'
        server=ThreadingHTTPServer(('127.0.0.1',0),NativeFixture)
        Thread(target=server.serve_forever,daemon=True).start()
        endpoint=f'http://127.0.0.1:{server.server_port}/v1'
        cases=experiment.benchmark.load_json(dataset)['cases']
        selected=[]
        for subject in experiment.SUBJECT_NAMES:
            selected.append(next(case['id'] for case in cases if case['subject']==subject and not case['turns']))
        report={'paidModelRequests':0,'realTeachingQualityEvaluated':False,'checks':[]}
        try:
            def plan(name,ids=selected,tokens=5000000,requests=100):
                folder=root/name
                experiment.prepare(dataset,driver,folder,ids,limit_tokens=tokens,limit_requests=requests)
                return folder
            preflight=plan('preflight',ids=None)
            status=experiment.execute(preflight)
            assert status['finishedCases']==108 and status['budget']['httpRequests']==0,status
            assert not status['allTurnsExecuted'] and status['qualityMetrics'] is None
            report['preflight']=status;report['checks'].append('108例零调用，分清已观察与预期轮数')
            folder=plan('nine-subjects')
            status=experiment.execute(folder,'fixture',endpoint)
            assert status['status']=='completed' and status['budget']['httpRequests']==len(self_calls:=NativeFixture.calls),status
            assert status['budget']['usedTokens']==9*len(self_calls) and status['allTurnsExecuted'],status
            for row in experiment.benchmark.load_json(folder/'run.json')['records']:
                result=experiment.benchmark.load_json(folder/row['directory']/'result.json')
                journal=[json.loads(line) for line in (folder/row['directory']/'result.json.responses.jsonl').read_text(encoding='utf-8').splitlines()]
                chunks=[]
                for item in journal:
                    if item.get('kind')=='chunk':chunks.append(item['text'])
                    else:
                        assert ''.join(chunks)==item['rawResponse'];chunks=[]
                assert not chunks
                assert journal[-1]['rawResponse'] in NativeFixture.replies
                assert result['promptAudit'][0]['messages']
            before=len(NativeFixture.calls);again=experiment.execute(folder,'fixture',endpoint)
            assert again['status']=='completed' and len(NativeFixture.calls)==before
            report['nineSubjects']=status;report['checks'].append('九科真实主控/SSE/用量/逐片段与原文/完成后无重复请求')
            multi=plan('multi-turn',[next(case['id'] for case in cases if case['turns'])])
            multi_status=experiment.execute(multi,'fixture',endpoint)
            assert multi_status['status']=='completed' and multi_status['observedTurns']>=2,multi_status
            report['multiTurn']=multi_status;report['checks'].append('用户历史加当前输入的多轮原生重放')
            before=len(NativeFixture.calls)
            # 损坏已完成文件必须在发请求前拒绝，不能跳过或自动重试。
            result_path=folder/'runs/000/result.json';original=result_path.read_bytes();result_path.write_bytes(b'{}')
            try:experiment.execute(folder,'fixture',endpoint)
            except ValueError:pass
            else:raise AssertionError('损坏结果未阻止恢复')
            assert len(NativeFixture.calls)==before;result_path.write_bytes(original)
            report['checks'].append('已有结果损坏时零新增请求')
            for name,tokens,requests in [('http-stop',5000000,1),('token-stop',1,100)]:
                limited=plan(name,selected[:1],tokens,requests);before=len(NativeFixture.calls)
                outcome=experiment.execute(limited,'fixture',endpoint)
                assert outcome['status']=='failed' and len(NativeFixture.calls)==before,outcome
                assert outcome['uncertainCases']==[] and outcome['finishedCases']==0,outcome
            report['checks'].append('HTTP与Token上限均在发送前拦截')
            NativeFixture.behavior='compatibility';compatible=plan('compatibility',selected[:1]);before=len(NativeFixture.calls)
            outcome=experiment.execute(compatible,'fixture',endpoint)
            assert outcome['status']=='completed' and outcome['budget']['httpRequests']==len(NativeFixture.calls)-before,outcome
            assert outcome['budget']['unknownUsageRequests']>0 and outcome['budget']['reservedTokens']>0,outcome
            report['checks'].append('用量参数兼容重试计入HTTP，未知用量保留额度')
            NativeFixture.behavior='timeout';timed=plan('timeout',selected[:1]);before=len(NativeFixture.calls)
            outcome=experiment.execute(timed,'fixture',endpoint,timeout=1)
            assert outcome['status']=='uncertain' and outcome['budget']['reservedTokens']>0,outcome
            assert (timed/'runs/000/result.json.responses.jsonl').stat().st_size>0
            experiment.execute(timed,'fixture',endpoint,timeout=1)
            assert len(NativeFixture.calls)==before+1
            report['checks'].append('中途终止保留片段和预留，uncertain不重发')
            NativeFixture.behavior='redirect';redirect=plan('redirect',selected[:1]);before=len(NativeFixture.calls)
            outcome=experiment.execute(redirect,'fixture',endpoint)
            assert outcome['status']=='failed' and len(NativeFixture.calls)>before,outcome
            report['checks'].append('本机模拟重定向拒绝，失败不记完成')
            report['passed']=True
            experiment.atomic_save(root/'report.json',report)
            print(json.dumps(report,ensure_ascii=False,indent=2))
        finally:server.shutdown();server.server_close()


if __name__=='__main__':
    main(Path(sys.argv[1]).resolve(),Path(sys.argv[2]).resolve() if len(sys.argv)>2 else None)
