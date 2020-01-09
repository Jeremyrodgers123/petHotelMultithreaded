//
//  FairHotel.hpp
//  PetHotel
//
//  Created by Jeremy Rodgers on 4/4/19.
//  Copyright © 2019 Jeremy Rodgers. All rights reserved.
//

#ifndef FairHotel_hpp
#define FairHotel_hpp

#include <stdio.h>
#include <iostream>
#include <stdio.h>
#include <chrono>
#include <thread>
#include<atomic>
#include <algorithm>
class FairHotel{
    
    /**
     The scheduler is originally unfair to cats. Birds and Dogs can come in to the hotel and play when there are birds or dogs in the hotel. This allows multiple bird/dogs to enter. All it takes for a cat not to enter is there to be 1 bird or 1 dog in the hotel. Birds come in first, let in a dog, and cats just sit and wait to be let in. Once the hotel closes all the cats can get in likely once.
     **/
    std::mutex mutex_;
    std::condition_variable noCats, noBirdsOrDogs;
    bool evictDogsAndBirds;
    bool evictCats;
    
    int runningTotalCats;
    int runningTotalDogs;
    int runningTotalBirds;
    
    int waitingDogs;
    int waitingCats;;
    int waitingBirds;
    
    std::atomic<bool> allowEvicting;
    std::atomic<bool> debug;
    bool blockBirds = false;
    bool blockDogs = false;
    int birdCount;
    int catCount;
    int dogCount;
    
    int expectedCatPer = 0;
    int expectedDogBirdPer = 0;
    //DECLARE THIS AS A PRIVATE METHOD IN YOUR HOTEL CLASS
    void play() const {
        for(volatile int i = 0; i < 10000; ++i) {} //use the CPU for a while
        std::this_thread::sleep_for(std::chrono::milliseconds(1)); //take a nap
    }
public:
    double birdArg = 0;
    double catArg = 0;
    double dogArg = 0;
    double animalCount = 0;
    
   
    std::atomic<bool> done;
    FairHotel(){
        birdCount = 0;
        catCount = 0;
        dogCount = 0;
        runningTotalCats = 0;
        runningTotalDogs = 0;
        runningTotalBirds = 0;
        waitingBirds = 0;
        waitingDogs = 0;
        waitingCats = 0;
        done = false;
        allowEvicting = false;
        debug = false;
    }
    
    bool catsWaiting(){
        if (waitingCats == 0 || catArg == 0){
            return false;
        }
        return true;
    }
    
    bool dogsOrBirdsWaiting(){
        if ( ( waitingDogs == 0 && waitingBirds == 0 ) ||
            (dogArg == 0 && birdArg == 0) ){
            return false;
        }
        return true;
    }
    
    double calcRTPercent(int animalRT){
        double runningTotal =  animalRT / (runningTotalCats + runningTotalDogs + runningTotalBirds);
        return runningTotal;
    }
    
    double petArrivalPercent(double & animalArg){
        double expected =  animalArg / animalCount;
        return expected;
    }
    
    bool isFair(double &runningTotalPercent, const double &expectedPercent){
        double upperBound = expectedPercent * 1;
        double lowerBound = expectedPercent * .90;
       
        if ((runningTotalPercent > lowerBound) && (runningTotalPercent < upperBound) ){
            return true;
        }
        return false;
    }
    
    void checkDogBirdsFair(){
        if((runningTotalBirds > (runningTotalDogs * 2)) && (runningTotalBirds > 50) ){
            blockBirds = true;
        }else if ( ((runningTotalBirds * 2) < runningTotalDogs) && runningTotalDogs > 50){
            blockDogs = true;
        }
        
    }
    
    double min( const double & a, const double & b ){
        if( a < b) return a;
        return b;
    }
    
    bool wannaBeFair(){
        if(done){
            evictCats = false;
            evictDogsAndBirds = false;
            allowEvicting = false;
            blockDogs = false;
            blockBirds = false;
            return false;
        }
        if( !catsWaiting() && (dogCount + birdCount > 0)){
            return false;
        }
        if( !dogsOrBirdsWaiting()  && (catCount > 0) ){
            return false;
        }
        if(!allowEvicting){
            double dogsBirdsRTPerc = calcRTPercent(runningTotalDogs + runningTotalBirds);
            double catsRTPerc = calcRTPercent(runningTotalCats);
            double catThreshold = min((catArg / 2), 10);
            double dogBirdThreshold = min(((dogArg + birdArg) / 2), 10);
            if( (catsWaiting())  && (! isFair(dogsBirdsRTPerc, expectedDogBirdPer ) ) ){
                allowEvicting = true;
                return true;
            }else if(( (waitingDogs + waitingBirds) > dogBirdThreshold )&& (! isFair(catsRTPerc, expectedCatPer))){
                allowEvicting = true;
                return true;
            }
            //checkDogBirdsFair();
            return false;
        }
        //checkDogBirdsFair();
        return true;
    }
    
    bool dogsBirdsWaiting(){
        if ( ( (waitingDogs + waitingBirds) == 0) || done){
            return false;
        }
        return true;
    }
    
    void calcTotalAnimals(){
        animalCount = birdArg + catArg + dogArg;
        double dogBirdArg = dogArg + birdArg;
        expectedCatPer = petArrivalPercent(catArg);
        expectedDogBirdPer= petArrivalPercent(dogBirdArg);
    }
    
    
    void bird(){
        std::unique_lock<std::mutex> l(mutex_);
        while(catCount > 0 || evictDogsAndBirds || blockBirds){
            waitingBirds += 1;
            noCats.wait(l);
            waitingBirds -= 1;
        }
        assert(catCount == 0);
        birdCount++;
        runningTotalBirds++;
        if( wannaBeFair() ){
            evictDogsAndBirds = true;
            blockBirds = false;
            blockDogs = false;
        }
        l.unlock();
        
        play();
        std::unique_lock<std::mutex> l2(mutex_);
        --birdCount;
        if ((birdCount + dogCount) == 0 ){
            //allowEvicting = false;
            noBirdsOrDogs.notify_all();
            evictDogsAndBirds = false;
        }
        
        l2.unlock();
        
    }
    
    void cat(){
        std::unique_lock<std::mutex> l(mutex_);
        while( ((birdCount + dogCount)> 0) || evictCats ){
            waitingCats += 1;
            noBirdsOrDogs.wait(l);
            waitingCats -= 1;
        }
        assert(dogCount + birdCount == 0);
        catCount++;
        runningTotalCats++;
        if( wannaBeFair()){
            evictCats = true;
        }
        l.unlock();
        
        play();
        
        std::unique_lock<std::mutex> l2(mutex_);
        --catCount;
        if(catCount == 0){
            allowEvicting = false;
            noCats.notify_all();
            evictCats = false;
        }
        
        l2.unlock();
    }
    
    void dog(){
        //        {
        std::unique_lock<std::mutex> l(mutex_);
        while(catCount > 0 || evictDogsAndBirds || blockDogs ){
            waitingDogs += 1;
            noCats.wait(l);
            waitingDogs -= 1;
        }
        assert(catCount == 0);
        dogCount++;
        runningTotalDogs++;
        if( wannaBeFair() ){
            evictDogsAndBirds = true;
            blockDogs = false;
            blockBirds = false;
        }
        l.unlock();
        
        play();
        
        std::unique_lock<std::mutex> l2(mutex_);
        --dogCount;
        if ((birdCount + dogCount) == 0 ){
            //allowEvicting = false;
            noBirdsOrDogs.notify_all();
            evictDogsAndBirds = false;
            
        }
        
        l2.unlock();
        
    }
    
};
#endif /* FairHotel_hpp */
