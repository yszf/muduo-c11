#ifndef MUDUO_BASE_DATE_H
#define MUDUO_BASE_DATE_H

#include "muduo-c11/base/copyable.h"
#include "muduo-c11/base/Types.h"

struct tm;

namespace muduo {

    class Date : public copyable {
    public:
        typedef struct {
            int year; // [1900~2500]
            int month; // [1~12]
            int day; // [1~31]
        } YearMonthDay;

        static const int kDaysPerWeek = 7;
        static const int kJulianDayOf1970_01_01;

        Date() : julianDayNumber_(0) {}

        Date(int year, int month, int day);

        explicit Date(int julianDayNum) : julianDayNumber_(julianDayNum) {}

        explicit Date(const struct tm&);

        void swap(Date& that) {
            std::swap(julianDayNumber_, that.julianDayNumber_);
        }

        bool valid() const {
            return julianDayNumber_ > 0;
        }

        // converts to yyyy-mm--dd format
        string toIsoString() const;

        YearMonthDay yearMonthDay() const;

        int year() const {
            return yearMonthDay().year;
        }

        int month() const {
            return yearMonthDay().month;
        }

        int day() const {
            return yearMonthDay().day;
        }
        // [0~6]
        int weekDay() {
            return (julianDayNumber_ + 1) % kDaysPerWeek; 
        }

        int julianDayNumber() const {
            return julianDayNumber_;
        }

    private:
        int julianDayNumber_; // 儒略日
    };

    inline bool operator<(Date x, Date y) {
        return x.julianDayNumber() < y.julianDayNumber();
    }

    inline bool operator==(Date x, Date y) {
        return x.julianDayNumber() == y.julianDayNumber();
    }

} // namespace muduo

#endif // MUDUO_BASE_DATE_H