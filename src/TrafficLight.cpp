#include <iostream>
#include <random>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */

template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function.
    // Locking the messageQueue so that no other thread accesses the message queue
    std::unique_lock<std::mutex> retrivalLock(_messageQueueMut);

    //Waiting for a response from the send function about an item being placed onthe message queue
    _condition.wait(retrivalLock);

    
    //Once an item is placed on the messaging queue, the waiting function returns and the new obejct can be retrieved from the message queue
    T retreiveditem = std::move(_queue.front());
    _queue.pop_front();
    
    //Returning the retrieved item
    return retreiveditem;
}


template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    //Putting a lock_guard on the queue so that no other thread modifies the queue
    std::lock_guard<std::mutex> insertionLock(_messageQueueMut);

    //Place the new message at the back of the message queue
    _queue.emplace_back(msg);

    //notifying that new information has been placed in the queue
    _condition.notify_one();
}

/* Implementation of class "TrafficLight" */

TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

TrafficLight::~TrafficLight() {}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
    
    //Implementing the infinite loop
    while (true)
    {
        //Calling the receive function of the message queue
        TrafficLightPhase receivedPhase = _messageQueue.receive();

        if(receivedPhase == TrafficLightPhase::green) {
            return;
        }
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class. 
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 

    long cycleDuration = generateCycleDuration(4000, 6000);

    //init stop watch
    std::chrono::time_point<std::chrono::system_clock> lastUpdate = std::chrono::system_clock::now();

    //infinite loop
    while(true) {
        
        //compute time difference to stop watch
        long timeSinceLastUpdate = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - lastUpdate).count();

        if(timeSinceLastUpdate >= cycleDuration) {
            
            //Toggle between red and green
            if(_currentPhase == TrafficLightPhase::red) {
                _currentPhase = TrafficLightPhase::green;
            } else {
                _currentPhase = TrafficLightPhase::red;
            }

            //send update message to queue
            _messageQueue.send(std::move(_currentPhase));
            
            //Computing new cycleDuration
            cycleDuration = generateCycleDuration(4000, 6000);

            //reset stop watch
            lastUpdate = std::chrono::system_clock::now();

        }

        //To optimze CPU utilization
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

    }
}

long TrafficLight::generateCycleDuration(long min, long max) {
    std::random_device ranDevice;
    std::mt19937 rng(ranDevice());
    std::uniform_real_distribution<> uni(min, max);

    long randNum = uni(rng);

    return randNum;
}