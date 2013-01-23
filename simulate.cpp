#include <iostream>
#include <cstdlib>
#include <string>
#include <sstream>

using namespace std;

int main()
{
    int n_max = 0;
    int n_min = 0;
    int stickiness = 0;
    int stageStickiness = 0;
    int time = 0;
    int fairShare = 0;
    double b_min = 220000;
    float error = 0.0;
    float drift = 0.0;
    
    stringstream command (stringstream::in | stringstream::out);
    
    cout << "This simulation will be executed as many times as n_max - n_min, where n are stations" << endl;
    cout << "Please introduce n_max" << endl;
    cin >> n_max;
    cout << "Please introduce n_min" << endl;
    cin >> n_min;
    cout << "How long will the simulation run? (s)" << endl;
    cin >> time;
    cout << "Initial bandwidth?" << endl;
    cin >> b_min;
    cout << "Stickiness?" << endl;
    cin >> stickiness;
    cout << "Stage Stickiness?" << endl;
    cin >> stageStickiness;
    cout << "fairShare?" << endl;
    cin >> fairShare;
    cout << "Channel error prob (%)" << endl;
    cin >> error;
    cout << "Slot drift (%)" << endl;
    cin >> drift;
    
    for(int i = n_min; i <= n_max; i++)
    {
        //execute script   
        command << "./Sim_SlottedCSMA" << " " << time << " " << i << " 1024 " << b_min << " 1 " << stickiness << " " << stageStickiness << " " << fairShare << " " << error << " " << drift << endl;
        //cout << command.str() << endl;
        cout << "Trying with " << i << " stations." << endl;
        system(command.str().c_str());
        command.str("");
    }
}
