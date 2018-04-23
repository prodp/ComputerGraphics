#include <string>
#include <vector>
#include <iostream>

#define     SIZE_LAST_STEPS     50

class SpeedTest {
    private:
        float start;
        float end;
        std::vector<string> labels;
        std::vector<float> steps;
        float lastSteps[SIZE_LAST_STEPS];
        int curIndex;
        int lastStepsActiveNb;

    public:
        SpeedTest() {
            for (int i = 0 ; i < SIZE_LAST_STEPS ; ++i) {
                lastSteps[i] = 0.0;
            }
            curIndex = 0;
            lastStepsActiveNb = 0;
            steps.reserve(32);
            labels.reserve(32);
        }

        // Begin a loop
        void begin() {
            start = glfwGetTime();
            steps.clear();
            labels.clear();
        }

        void step(string label) {
            float currentTime = glfwGetTime();
            steps.push_back(currentTime);
            labels.push_back(label);
        }

        // Finish a loop
        void finish() {
            this->end = glfwGetTime();
            lastSteps[curIndex] = end - start;
            curIndex++;
            if (curIndex >= SIZE_LAST_STEPS) {
                curIndex = 0;
            }
            if (lastStepsActiveNb < SIZE_LAST_STEPS) {
                lastStepsActiveNb++;
            }
        }

        void display() {
            for (int i = 0 ; i < steps.size() ; ++i) {
                if (i == 0) {
                    cout << labels[i] << " : " << steps[i] - start << endl;
                }
                else {
                    cout << labels[i] << " : " << steps[i] - steps[i-1] << endl;
                }
            }
            float mean = 0.0;
            for (int i = 0 ; i < SIZE_LAST_STEPS ; ++i) {
                if (lastSteps[i] > 0.0) {
                    mean += lastSteps[i];
                }
            }
            //cout << "TOTAL TIME : " << end - start << endl;
            cout << "FPS : " << 1 / (mean/lastStepsActiveNb) << endl;
            cout << "Time per loop : " << (mean/lastStepsActiveNb) << endl;
            //cout << "**********************************" << endl;
        }
};
