#include <iostream>
#include <fstream>
#include "../includes/lengthOfFile.hh"

using namespace std;

struct container{
    float time;
    double cp;
    double stdCP; //standard deviation
    int readings;
};

int main()
{
    container *readings; 
    int length = 0;
    string line;
    int iterator = 0;
    double stdNumerator = 0;
    
    ofstream averageCollisionsInTime;

    ifstream inputFile;
    
    //Just to measure the length of the file
    inputFile.open("collisionsInTime.txt");
    while(!inputFile.eof())
    {
        getline(inputFile, line);
        length++;
    }
    inputFile.close();
    readings = new container[length];

    cout << "Length of the file: " << length << endl;
    /////////////////////////////////////////
    
    //Doing the actual reading
    inputFile.open("collisionsInTime.txt");
    
    if(!inputFile)
    {
        cout << "File could not be opened" << endl;
    }else
    {
        while(!(inputFile.eof()))
        {
            inputFile >> (readings[iterator].time) >> (readings[iterator].cp);
            //cout << (readings[iterator].time) << " " << (readings[iterator].cp) << endl;
            iterator++;
        }
    }
    

    //10 is the number of seconds of the simulation
    container average [10] = {};
    int newTime = 0;
    int traveller = 0;

    while(traveller != length)
    {
        if((int)(readings[traveller].time) == newTime)
        {
            cout << (int)(readings[traveller].time) << " is equal to " << newTime << endl;
            (average[newTime].time) = newTime;
            (average[newTime].cp) += (readings[traveller].cp);
            (readings[newTime].readings)+=1;
            //cout << "Storing the new time, the cp and the readings " << (average[newTime].time) << " " << (average[newTime].cp) << " " << (readings[newTime].readings) << endl;
            traveller++;
        }else
        {
            //cout << (int)(readings[traveller].time) << " is NOT equal to " << newTime << endl;
            //cout << "Changing the value of newTime to: " << (int)(readings[traveller].time) << endl;
            newTime = (int)(readings[traveller].time);
        } 
    }
    
    cout << "Completed the reorganization of the readings" << endl;
        
    
    
    averageCollisionsInTime.open("averageCollisionsInTime.txt", ios::app);
    
    //Arranging the average array
    for(int k = 0; k < 10; k++)
    {
        cout << readings[k].readings << endl;
        (average[k].cp) = (average[k].cp)/(readings[k].readings);
    }
    
    //Computing the standard deviation of the cp
    newTime = 0;
    traveller = 0;
    
    while(traveller != length)
    {
        if((int)(readings[traveller].time) == newTime)
        {
            stdNumerator += pow((average[newTime].cp)-(readings[traveller].cp),2);
            traveller++;
        }else
        {
            //cout << stdNumerator << endl;
            (average[newTime].stdCP) = sqrt((1./(readings[newTime].readings))*stdNumerator);
            stdNumerator = 0;
            newTime = (int)(readings[traveller].time);
        }
    }
    
    //output to file
    for(int m = 0; m < 10; m++)
    {
        averageCollisionsInTime << average[m].time << " " << average[m].cp << " " << average[m].stdCP << endl;
    }
            
    
    averageCollisionsInTime.close();
    
}
