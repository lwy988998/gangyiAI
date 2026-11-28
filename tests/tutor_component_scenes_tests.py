"""组件候选的错配、合成来源和原生事件转换边界，不发模型请求。"""
import copy
from pathlib import Path
import sys
import unittest

ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'scripts'))
import tutor_benchmark as benchmark
import tutor_component_scenes as scenes
import tutor_experiment as experiment


class ComponentSceneTests(unittest.TestCase):
    def setUp(self):
        self.dataset=benchmark.load_json(ROOT/'tests/fixtures/ai-tutor/candidate-v3.json')
        self.case=copy.deepcopy(next(case for case in self.dataset['cases'] if case['id']=='mathematics-v3-interval'))

    def test_revision_preserves_original_questions_and_marks_all_evidence_synthetic(self):
        old=benchmark.load_json(ROOT/'tests/fixtures/ai-tutor/candidate-v2.json')
        self.assertEqual(benchmark.validate_dataset(self.dataset)['cases'],112)
        for before,after in zip(old['cases'],self.dataset['cases']):
            for key,value in before.items():
                if key!='componentTypes':self.assertEqual(after[key],value)
        for case in self.dataset['cases']:
            scenes.validate_scene_plan(case)
            self.assertEqual(case['componentPlan']['evidenceSource'],'synthetic_fixture')
        self.assertEqual(self.dataset['review']['status'],'pending_expert_review')

    def test_static_diagrams_and_unsupported_goals_are_not_relabelled_interactive(self):
        cases={case['id']:case for case in self.dataset['cases']}
        for cid in ['physics-force-03','biology-ecology-08','history-timeline-09']:
            self.assertEqual(cases[cid]['componentPlan']['mode'],'static')
            self.assertEqual(cases[cid]['componentTypes'],[])
        for cid in ['computing-loop-17','chinese-reexplain-14','chinese-v2-10']:
            self.assertEqual(cases[cid]['componentPlan']['mode'],'text_fallback')

    def test_label_static_mode_and_fabricated_provenance_mismatches_are_rejected(self):
        for patch in [{'mode':'static'},{'staticTypes':['timeline']},{'evidenceSource':'real_student'}]:
            bad=copy.deepcopy(self.case);bad['componentPlan'].update(patch)
            with self.assertRaises(ValueError):scenes.validate_scene_plan(bad)
        bad=copy.deepcopy(self.case);bad['componentTypes']=['sets']
        with self.assertRaises(ValueError):scenes.validate_scene_plan(bad)

    def test_duplicate_identity_stale_state_and_injected_media_status_are_rejected(self):
        fixture=self.case['componentPlan']['fixtures'][0]
        bad=copy.deepcopy(self.case);bad['componentPlan']['fixtures'].append(copy.deepcopy(fixture))
        with self.assertRaises(ValueError):scenes.validate_scene_plan(bad)
        for version in [0,True,2]:
            bad=copy.deepcopy(self.case);bad['componentPlan']['fixtures'][0]['states'][0]['contentVersion']=version
            with self.assertRaises(ValueError):scenes.validate_scene_plan(bad)
        bad=copy.deepcopy(self.case);bad['componentPlan']['fixtures'][0]['config']['status']='failed'
        with self.assertRaises(ValueError):scenes.validate_scene_plan(bad)

    def test_only_current_event_receives_final_activity_and_conversion_does_not_mutate_source(self):
        self.case['turns']=[{'role':'user','text':'我想先理解开闭端点。'}]
        original=copy.deepcopy(self.case)
        native=experiment.native_case(self.case)
        self.assertNotIn('componentActivities',native['turns'][0])
        event=native['turns'][-1]
        trace=event['componentActivities']['scene-component']
        self.assertEqual(event['mediaState']['scene-component'],trace['events'][-1]['state'])
        self.assertEqual([row['seq'] for row in trace['events']],[1,2])
        self.assertNotIn('containsPoint',trace['events'][-1]['state'])
        native['turns'][-1]['mediaState']['scene-component']['point']=20
        native['componentFixtures'][0]['states'][0]['point']=10
        self.assertEqual(self.case,original)
        self.assertIn('非真实学生操作',native['concept'])

    def test_offline_request_plan_carries_component_source_and_old_versions_still_validate(self):
        plan=benchmark.prepare(self.dataset)
        record=next(row for row in plan['records'] if row['caseId']==self.case['id'])
        self.assertEqual(record['input']['componentPlan'],self.case['componentPlan'])
        self.assertFalse(plan['executed'])
        old=benchmark.load_json(ROOT/'tests/fixtures/ai-tutor/seed-v1.json')
        self.assertEqual(benchmark.validate_dataset(old)['cases'],18)
        self.assertNotIn('componentPlan',benchmark.prepare(old)['records'][0]['input'])


if __name__=='__main__':unittest.main()
