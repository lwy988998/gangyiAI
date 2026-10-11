#include "mobile_policy.hpp"
#include <cassert>

int main() {
    using gangyi::MobilePolicy;
    MobilePolicy::configure(false);
    assert(MobilePolicy::allows("desktop"));
    assert(MobilePolicy::automaticWorkAllowed());
    MobilePolicy::configure(true);
    assert(!MobilePolicy::allows("unfinished"));
    assert(!MobilePolicy::automaticWorkAllowed());
    MobilePolicy::submitted("startup", "startup");
    assert(!MobilePolicy::allows("startup"));
    MobilePolicy::submitted("student", "plan_course");
    assert(MobilePolicy::allows("student"));
    assert(MobilePolicy::automaticWorkAllowed());
    MobilePolicy::setForeground(false, {"student", "request-in-flight"});
    assert(MobilePolicy::allows("student"));
    assert(MobilePolicy::requestAllowed("request-in-flight"));
    assert(!MobilePolicy::automaticWorkAllowed());
    MobilePolicy::submitted("unseen", "learning_updated");
    assert(!MobilePolicy::allows("unseen"));
    assert(!MobilePolicy::requestAllowed("unseen"));
    MobilePolicy::setForeground(true);
    MobilePolicy::resumed("unfinished");
    assert(MobilePolicy::allows("unfinished"));
    MobilePolicy::configure(true);
    assert(!MobilePolicy::allows("unfinished"));
    assert(!MobilePolicy::automaticWorkAllowed());
    MobilePolicy::configure(false);
}
