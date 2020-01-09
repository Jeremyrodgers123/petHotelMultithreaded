/**
 
 Bird
 
 **/
if(birdCount >= 5 || dogCount >= 5){
    noCats.wait(l);
}



/**
 
 Cat
 
 **/
if(catCount >= 5){
    noBirdsOrDogs.wait(l);
}
/**
 
 Dog
 
 **/

if(birdCount >= 5 || dogCount >= 5){
    noCats.wait(l);
}

if( (dogCount == 0) && (birdCount == 0)){
    noBirdsOrDogs.notify_all();
}

while(catCount > 0 || dogCount > 2 || birdCount > 2){
    if( (catCount == 0) && (dogCount == 0) && (birdCount == 0)){
        noBirdsOrDogs.notify_all();
    }
    noCats.wait(l);
}
/**
 
 General
 
 **/
l.lock();
std::atomic<int> birdCount;
std::atomic<int> catCount;
std::atomic<int> dogCount;
std::atomic<bool> evictDogsAndBirds;
std::atomic<bool> evictCats;
