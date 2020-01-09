//
//  PetHotel.hpp
//  PetHotel
//
//  Created by Jeremy Rodgers on 3/28/19.
//  Copyright © 2019 Jeremy Rodgers. All rights reserved.
//

#ifndef PetHotel_hpp
#define PetHotel_hpp

#include <stdio.h>
#include <chrono>
#include <thread>
#include<atomic>
class PetHotel{
    
    /**
     The scheduler is originally unfair to cats. Birds and Dogs can come in to the hotel and play when there are birds or dogs in the hotel. This allows multiple bird/dogs to enter. All it takes for a cat not to enter is there to be 1 bird or 1 dog in the hotel. Birds come in first, let in a dog, and cats just sit and wait to be let in. Once the hotel closes all the cats can get in likely once.
     **/
    std::mutex mutex_;
    std::condition_variable noCats, noBirdsOrDogs;
    std::atomic<int> birdCount;
    std::atomic<int> catCount;
    std::atomic<int> dogCount;
//    int birdCount = 0;
//    int catCount = 0;
//    int dogCount = 0;
    //DECLARE THIS AS A PRIVATE METHOD IN YOUR HOTEL CLASS
    void play() const {
        for(volatile int i = 0; i < 10000; ++i) {} //use the CPU for a while
        std::this_thread::sleep_for(std::chrono::milliseconds(1)); //take a nap
        
    }
public:
    PetHotel(){
        birdCount = 0;
        catCount = 0;
        dogCount = 0;
    }

    void bird(){
//        {
            std::unique_lock<std::mutex> l(mutex_);
            while(catCount > 0){
                noCats.wait(l);
            }
            assert(catCount == 0);
            birdCount++;
            l.unlock();
            //noBirdsOrDogs.notify_all();
//        }
        play();
         --birdCount;
         noBirdsOrDogs.notify_all();
    }
    
    void cat(){
//        {
            std::unique_lock<std::mutex> l(mutex_);
            while( (birdCount > 0) || (dogCount > 0) ){
                noBirdsOrDogs.wait(l);
            }
            assert(dogCount + birdCount == 0);
            catCount++;
            l.unlock();
            //noCats.notify_all();
//        }
       
        play();
        --catCount;
        noCats.notify_all();
    }
    
    void dog(){
//        {
            std::unique_lock<std::mutex> l(mutex_);
            while(catCount > 0){
                noCats.wait(l);
            }
            assert(catCount == 0);
            dogCount++;
            l.unlock();
           // noBirdsOrDogs.notify_all();
//        }
       
        play();
        --dogCount;
        noBirdsOrDogs.notify_all();
       
    }

};

#endif /* PetHotel_hpp */
