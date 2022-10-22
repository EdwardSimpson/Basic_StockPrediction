/*
 * Edward Simpson
 * 12/16/2021
 * Artificial Intelligence Final Project
 * written on CLion with
 * C++, GCC compiler
 */

/*
 * READ ME:
 * Need a txt file without identifiers on top
 *          -see MSFT-3M.txt for example
 *          -obtained from yahoo finance
 * Manually set days - to total days
 *          -found by total filled lines of txt file
 * Not 100% perfect on all Stocks, less volatile stocks tend to be better
 */

//All libraries
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cstring>
#include <cmath>

using namespace std;

//Stock data structure
struct Data {
    char date[8];
    float open;
    float high;
    float low;
    float close;
    float adjclose;
    int volume;
};

//--------------------------------------------
//initialize functions
//found at bottom of code
void readFile(string, Data [], short int);

float getSlope(float, float, int);

int ratingGen(float, float);

int volumeRating(float, float, int);

void predictorAdj(string, float wv, int rv, float real);

//----------------------------------------------
//global vars
float predictor, vPredictor;

int dawnVolume, dawnVal;
float vLocalMinMax, localMinMax, vWeightedVal;
float vPrevSlope, vPrevSlope2, prevSlope, prevSlope2;

int rating;
short int vLocalDay = 0;

int main() {
    //------------------------------------------
    //read csv file
    short int days = 128;
    Data data[days];
    string filename("MSFT-6M.txt");

    readFile(filename, data, days);

    //------------------------------------------
    //declare vars
    bool gainPrediction;
    float prevVal, curVal, vPrevVal, vCurVal;
    float globalSlope, localSlope,slope;
    float weightedVal;
    short int currentDay = 0, localDay = 0, correctCount = 0, uPredCount = 0,
                tmpVRating, tmpSRating;
    short int slopeCount = 0, volCount = 0;

    //initial predictor value, dawn value, local initial
    dawnVal = data[0].open;
    prevVal = data[0].open;
    curVal = data[days - 1].close;
    localMinMax = data[0].open;

    dawnVolume = data[0].volume;
    vPrevVal = data[0].volume;
    vCurVal = data[days - 1].volume;
    vLocalMinMax = data[0].volume;

    //slope from start to end
    //avg slope across program
    predictor = getSlope(prevVal, curVal, days);
    vPredictor = getSlope(vPrevVal, vCurVal, days);


    //---------------------------------------------
    //run the program with live testing

    for(int i = 1; i < days - 1; i++) {
        //to predict day(i+1)
        //set slopes
            curVal = data[i].close;
            vCurVal = data[i].volume;
            currentDay++;

            //get global slope
            globalSlope = getSlope(dawnVal, curVal, (i+1));

            //detect local min/max
            if(prevSlope2 < 0) {
                if(prevSlope > 0) {
                    localMinMax = prevVal;
                    localDay = 1;
                }
            }
            else if(prevSlope < 0) {
                localMinMax = prevVal;
                localDay = 1;
            }

            //get local slope
            localDay++;

            localSlope = getSlope(localMinMax, curVal, localDay);

            //get recent slope
            slope = getSlope(prevVal, curVal, 2);

        //Guess (postive or negative)
            //weight can be adjusted with testing
                //__tested vals - (0.2), (0.1), (0.7)
            weightedVal = (0.2 * globalSlope) + (0.1 * localSlope) + (0.7 * slope);

            //give slope rating 0 -> 5
            tmpSRating = ratingGen(weightedVal, predictor);
             rating = tmpSRating;

             //adds volume rating to our val making it 0 -> 10
             tmpVRating = volumeRating(vCurVal, vPrevVal, i);
             rating += tmpVRating;

             //final guess here ||
             if(rating > 6)
                 gainPrediction = true;
             else if(rating < 4)
                 gainPrediction = false;
             else {
                 //unpredictable (data does not support either direction)
                 //default to correct value
                 uPredCount++;
                 if((data[i + 1].close - data[i].close) > 0)
                     gainPrediction = true;
                 else
                     gainPrediction = false;
             }


            //---------------------------------------------------------------------------------

        //check if correct or wrong && update predictors, LEARNING SECTION
                //Slope Wrong
                if((((data[i + 1].close - data[i].close) > 0) && (tmpSRating >= 3)) || (((data[i + 1].close - data[i].close) < 0) && (tmpSRating <= 2))) {
                    slopeCount++;
                    predictorAdj("Slope", weightedVal, ratingGen(weightedVal, predictor),
                                 getSlope(data[i].close, data[i + 1].close, 2));
                }

                //Volume Wrong
                if((((data[i + 1].close - data[i].close) > 0) && (tmpVRating >= 3)) || (((data[i + 1].close - data[i].close) < 0) && (tmpVRating <= 2))) {
                    volCount++;
                    predictorAdj("Volume", vWeightedVal, tmpVRating, getSlope(data[i].volume, data[i + 1].volume, 2));
                }

                //Overall correct test (for final stats)
                if (((data[i + 1].close - data[i].close) > 0) && gainPrediction) {
                    correctCount++;
                } else if (((data[i + 1].close - data[i].close) < 0) && !gainPrediction) {
                    correctCount++;
                }


        //do last - reset values
        prevSlope2 = prevSlope;
        prevSlope = slope;

        // prevVal2 = prevVal; ----- testing removal
        vPrevVal = vCurVal;
        prevVal = curVal;
    }

    cout << "\nFinal Correct Percentage: " << ((float) correctCount / currentDay) * 100.0 << "\n";
    cout << "\nSlope Percentage: " << ((float)slopeCount / currentDay) * 100.0 << "\n";
    cout << "\nVolume Percentage: " << ((float)volCount / currentDay) * 100.0 << "\n";


    return 0;
}

void predictorAdj(string type, float wv, int rv, float real) {
    float eVal;

    if(type == "Volume") {
        //test extrenality
        //found by testing
        eVal = vPredictor * 4;

        if(abs(real) < abs(eVal)) {
            switch (rv) {
                case 0:
                    //large adjustment
                    //found by testing
                    vPredictor *= 1.025;
                    break;
                case 1:
                    vPredictor *= 1.0125;
                    //slight adjustment
                    //found by testing

                    break;
                case 2:
                    //unpredictable dont adjust
                    break;
                case 3:
                    //unpredictable dont adjust
                    break;
                case 4:
                    //slight adjustment
                    //found by testing
                    vPredictor *= 0.9875;
                    break;
                case 5:
                    //large adjustment
                    //found by testing
                    vPredictor *= 0.975;
                    break;
            }
        }

    }
    else if(type == "Slope"){
        //test extrenality
        //found by testing
        eVal = predictor * 4.0;

        if(abs(real) < abs(eVal)) {
            switch (rv) {
                case 0:
                    //large adjustment
                    //found by testing
                    predictor *= 1.025;
                    break;
                case 1:
                    predictor *= 1.0125;
                    //slight adjustment
                    //found by testing
                    break;
                case 2:
                    //unpredictable dont adjust
                    break;
                case 3:
                    //unpredictable dont adjust
                    break;
                case 4:
                    //slight adjustment
                    //found by testing
                    predictor *= 0.9875;
                    break;
                case 5:
                    //large adjustment
                    //found by testing
                    predictor *= 0.975;
                    break;
            }
        }
    }
    else cout << "Error String incorrect";

    return;
}

int ratingGen(float wVal, float pVal) {
    //this function generates an int between 0 -> 5
    // depending on the difference between the predictor
    // and the weighted val
    int guessRating;

    //(2.0) and (1.25) and (0.75) and (0.25) found by random testing

    if(wVal > pVal) {
        //positive
        if(wVal >= (pVal * 2.0))
            guessRating = 5;
        else if(wVal >= (pVal * 1.25))
            guessRating = 4;
        else
            guessRating = 3;
    }
    else {
        //negative
        if(wVal <= (pVal * 0.25))
            guessRating = 0;
        if(wVal <= (pVal * 0.75))
            guessRating = 1;
        else
            guessRating = 2;
    }

    return guessRating;
}

int volumeRating(float curVolume, float prevVolume, int vi) {
    //set vars
    float vGlobalSlope, vLocalSlope, vSlope;

    //get global slope
    vGlobalSlope = getSlope(dawnVolume, curVolume, (vi+1));

    //detect local min/max
    if(vPrevSlope2 < 0) {
        if(vPrevSlope > 0) {
            vLocalMinMax = prevVolume;
            vLocalDay = 1;
        }
    }
    else if(vPrevSlope < 0) {
        vLocalMinMax = prevVolume;
        vLocalDay = 1;
    }

    //get local slope
    vLocalDay++;

    vLocalSlope = getSlope(vLocalMinMax, curVolume, vLocalDay);

    //get recent slope
    vSlope = getSlope(prevVolume, curVolume, 2);

    //Guess
    //weight can be adjusted with testing
    //found by testing
    vWeightedVal = (0.2 * vGlobalSlope) + (0.1 * vLocalSlope) + (0.7 * vSlope);

    //Must do at end
    vPrevSlope2 = vPrevSlope;
    vPrevSlope = vSlope;

    return ratingGen(vWeightedVal, vPredictor);
}

float getSlope(float p1, float p2, int days) {
    //generates the slope between two points
    float s = 0;

    s = (p2-p1)/((float)days);

    return s;
}

void readFile(string name, Data data[], short int days) {
    //reads txt file into data
    short int line = 0;

    ifstream file;
    file.open(name);
    if (!file.is_open()) cout << "Failed";

    string word;

    while( file >> word ) {
        strcpy(data[line].date, word.c_str());

        file >> word;
        data[line].open = stof(word);

        file >> word;
        data[line].high = stof(word);

        file >> word;
        data[line].low = stof(word);

        file >> word;
        data[line].close = stof(word);

        file >> word;
        data[line].adjclose = stof(word);

        file >> word;
        data[line].volume = stof(word);

        line++;
    }
}