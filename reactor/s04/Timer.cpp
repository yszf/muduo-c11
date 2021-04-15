#include "Timer.h"

using namespace muduo;

Timer::Timer(const TimerCallback& cb, Timestamp when, double interval) 
    : callback_(cb), 
    expiration_(when),
    interval_(interval),
    repeat_(interval > 0.0){

}

Timer::~Timer() {

}

void Timer::restart(Timestamp now) {
    if (repeat_) {
        expiration_ = addTime(now, interval_);
    }
    else {
        expiration_ = Timestamp::invalid();
    }
}