/**
 * @file observer_list..h
 * @brief observer patern
 * @author liangsijian
 * @version 
 * @date 2015-04-26
 */

#ifndef OBSERVER_LIST_H_QVPXZT6K
#define OBSERVER_LIST_H_QVPXZT6K

#include <limits>
#include <vector>
#include <algorithm>
#include <assert.h>
///////////////////////////////////////////////////////////////////////////////
//
// OVERVIEW:
//
//   A container for a list of observers.  Unlike a normal STL vector or list,
//   this container can be modified during iteration without invalidating the
//   iterator.  So, it safely handles the case of an observer removing itself
//   or other observers from the list while observers are being notified.
//
// TYPICAL USAGE:
//
//   class MyWidget {
//    public:
//     ...
//
//     class Observer {
//      public:
//       virtual void OnFoo(MyWidget* w) = 0;
//       virtual void OnBar(MyWidget* w, int x, int y) = 0;
//     };
//
//     void AddObserver(Observer* obs) {
//       observer_list_.AddObserver(obs);
//     }
//
//     void RemoveObserver(Observer* obs) {
//       observer_list_.RemoveObserver(obs);
//     }
//
//     void NotifyFoo() {
//       FOR_EACH_OBSERVER(Observer, observer_list_, OnFoo(this));
//     }
//
//     void NotifyBar(int x, int y) {
//       FOR_EACH_OBSERVER(Observer, observer_list_, OnBar(this, x, y));
//     }
//
//    private:
//     ObserverList<Observer> observer_list_;
//   };
//
//
///////////////////////////////////////////////////////////////////////////////

template <class ObserverType, bool check_empty = false>
class ObserverList {
public:
    // Enumeration of which observers are notified.
    enum NotificationType {
        // Specifies that any observers added during notification are notified.
        // This is the default type if non type is provided to the constructor.
        NOTIFY_ALL,

        // Specifies that observers added while sending out notification are not
        // notified.
        NOTIFY_EXISTING_ONLY
    };

    ObserverList() : notify_depth_(0), type_(NOTIFY_ALL) {}
    ObserverList(NotificationType type) : notify_depth_(0), type_(type) {}
    ~ObserverList() {
        // When check_empty is true, assert that the list is empty on destruction.
        if (check_empty) {
            Compact();
            assert(observers_.size() == 0U);
        }
    }

    // Add an observer to the list.
    void AddObserver(ObserverType* obs) {
        assert(find(observers_.begin(), observers_.end(), obs) == observers_.end());
        observers_.push_back(obs);
    }

    // Remove an observer from the list.
    void RemoveObserver(ObserverType* obs) {
        typename ListType::iterator it =
            std::find(observers_.begin(), observers_.end(), obs);
        if (it != observers_.end()) {
            if (notify_depth_) {
                *it = 0;
            }
            else {
                observers_.erase(it);
            }
        }
    }

    size_t size() const {
        return observers_.size();
    }

    ObserverType* GetElementAt(int index) const {
        return observers_[index];
    }

    // An iterator class that can be used to access the list of observers.  See
    // also the FOREACH_OBSERVER macro defined below.
    class Iterator {
    public:
        Iterator(const ObserverList<ObserverType>& list)
            : list_(list),
            index_(0),
            max_index_(list.type_ == NOTIFY_ALL ?
            std::numeric_limits<size_t>::max() :
            list.observers_.size()) {
            ++list_.notify_depth_;
        }

        ~Iterator() {
            if (--list_.notify_depth_ == 0)
                list_.Compact();
        }

        ObserverType* GetNext() {
            ListType& observers = list_.observers_;
            // Advance if the current element is null
            size_t max_index = std::min(max_index_, observers.size());
            while (index_ < max_index && !observers[index_])
                ++index_;
            return index_ < max_index ? observers[index_++] : NULL;
        }

    private:
        const ObserverList<ObserverType>& list_;
        size_t index_;
        size_t max_index_;
    };

private:
    typedef std::vector<ObserverType*> ListType;

    void Compact() const {
        typename ListType::iterator it = observers_.begin();
        while (it != observers_.end()) {
            if (*it) {
                ++it;
            }
            else {
                it = observers_.erase(it);
            }
        }
    }

    // These are marked mutable to facilitate having NotifyAll be const.
    mutable ListType observers_;
    mutable int notify_depth_;
    NotificationType type_;

    friend class ObserverList::Iterator;

    ObserverList(const ObserverList&);
    void operator = (const ObserverList&);
    
};

#define FOR_EACH_OBSERVER(ObserverType, observer_list, func)  \
do { \
    ObserverList<ObserverType>::Iterator it(observer_list);   \
    ObserverType* obs;                                        \
while ((obs = it.GetNext()) != NULL)                      \
    obs->func;                                              \
} while (0)

class Foo {
public:
    virtual void Observe(int x) = 0;
    virtual ~Foo() {}
};

class Adder : public Foo {
public:
    explicit Adder(int scaler) : total(0), scaler_(scaler) {}
    virtual void Observe(int x) {
        total += x * scaler_;
    }
    virtual ~Adder() { }
    int total;
private:
    int scaler_;
};

class Disrupter : public Foo {
public:
    Disrupter(ObserverList<Foo>* list, Foo* doomed)
        : list_(list), doomed_(doomed) { }
    virtual ~Disrupter() { }
    virtual void Observe(int x) {
        list_->RemoveObserver(doomed_);
    }
private:
    ObserverList<Foo>* list_;
    Foo* doomed_;
};

class AddInObserve : public Foo {
public:
    AddInObserve(ObserverList<Foo>* observer_list)
        : added(false),
        observer_list(observer_list),
        adder(1) {
    }
    virtual void Observe(int x) {
        if (!added) {
            added = true;
            observer_list->AddObserver(&adder);
        }
    }

    bool added;
    ObserverList<Foo>* observer_list;
    Adder adder;
};


#endif /* end of include guard: OBSERVER_LIST_H_QVPXZT6K */
