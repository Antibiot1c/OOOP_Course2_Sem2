#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <thread>
#include <mutex>
#include <algorithm>

using namespace std;

// Structure to store information about tasks
struct Task {
    string name;
    int max_score;
};

// Class to store and evaluate task results
class TaskManager {
private:
    vector<Task> tasks;
    map<string, vector<int>> scores; // Store scores for each student
    int num_threads; // Number of worker threads for parallelization

public:
    // Constructor
    TaskManager(int num_threads = thread::hardware_concurrency()) : num_threads(num_threads) {}

    // Add a task
    void add_task(string name, int max_score) {
        tasks.push_back({ name, max_score });
    }

    // Add scores for a student
    void add_scores(string student, vector<int> student_scores) {
        // Check if any of the scores are negative
        for (int score : student_scores) {
            if (score < 0) {
                cout << "Error: negative score detected. Please enter a non-negative score." << endl;
                return;
            }
        }
        // Add scores if all scores are non-negative
        scores[student] = student_scores;
    }

    // Calculate overall result for a student (in parallel)
    double calculate_result(string student) {
        vector<double> partial_scores(num_threads, 0.0); // Partial scores for each thread
        vector<thread> threads(num_threads);
        int num_tasks = min((int)scores[student].size(), (int)tasks.size());
        int tasks_per_thread = num_tasks / num_threads;
        mutex mtx; // Mutex to protect access to partial_scores

        // Start worker threads
        for (int i = 0; i < num_threads; i++) {
            threads[i] = thread([this, i, tasks_per_thread, num_tasks, &partial_scores, &mtx, student]() {
                int start = i * tasks_per_thread;
                int end = (i == num_threads - 1) ? num_tasks : (i + 1) * tasks_per_thread;
                vector<int> student_scores = scores[student];
                for (int j = start; j < end; j++) {
                    // Check and adjust scores
                    if (student_scores[j] < 0) {
                        student_scores[j] = 0;
                    }
                    else if (student_scores[j] > tasks[j].max_score) {
                        student_scores[j] = tasks[j].max_score;
                    }
                    // Add score to partial score
                    partial_scores[i] += student_scores[j];
                }
                });
        }

        // Join worker threads
        for (auto& thread : threads) {
            thread.join();
        }

        // Calculate final score
        double total_score = 0.0;
        for (double score : partial_scores) {
            total_score += score;
        }

        // Return average score
        return total_score / tasks.size();
    }

    // Get scores for all students
    map<string, vector<int>> get_scores() const {
        return scores;
    }

    // Get number of tasks
    int num_tasks() const {
        return tasks.size();
    }

    // Get names of all tasks
    vector<string> get_task_names() const {
        vector<string> task_names;
        for (Task task : tasks)
        {
            task_names.push_back(task.name);
        }
        return task_names;
    }

    // Get maximum scores for all tasks
    vector<int> get_max_scores() const {
        vector<int> max_scores;
        for (Task task : tasks) {
            max_scores.push_back(task.max_score);
        }
        return max_scores;
    }

    // Get names of all students
    vector<string> get_student_names() const {
        vector<string> student_names;
        for (auto& score : scores) {
            student_names.push_back(score.first);
        }
        return student_names;
    }

    // Get score for a specific task and student
    int get_score(string student, string task) const {
        auto it = find_if(tasks.begin(), tasks.end(), [task](const Task& t) {
            return t.name == task;
            });
        if (it == tasks.end()) {
            cout << "Error: task not found." << endl;
            return -1;
        }
        int task_index = it - tasks.begin();
        auto it2 = scores.find(student);
        if (it2 == scores.end()) {
            cout << "Error: student not found." << endl;
            return -1;
        }
        vector<int> student_scores = it2->second;
        if (task_index >= student_scores.size()) {
            cout << "Error: student has not completed this task." << endl;
            return -1;
        }
        return student_scores[task_index];
    }
};

int main() {
    TaskManager task_manager(4); // Use 4 worker threads for parallelization
    task_manager.add_task("Task 1", 10);
    task_manager.add_task("Task 2", 8);
    task_manager.add_task("Task 3", 6);

    task_manager.add_scores("Student 1", { 8, 7, 6 });
    task_manager.add_scores("Student 2", { 9, 5, 4 });
    task_manager.add_scores("Student 3", { 10, 9, 6 });
    task_manager.add_scores("Student 4", { 7, 5, 6 });

    cout << "Number of tasks: " << task_manager.num_tasks() << endl;
    cout << "Task names: ";
    for (string name : task_manager.get_task_names()) {
        cout << name << " ";
    }
    cout << endl;

    cout << "Maximum scores: ";
    for (int max_score : task_manager.get_max_scores()) {
        cout << max_score << " ";
    }
    cout << endl;

    cout << "Number of students: " << task_manager.get_student_names().size() << endl;

    cout << "Score for Student 1, Task 1: " << task_manager.get_score("Student 1", "Task 1") << endl;
    cout << "Score for Student 1, Task 2: " << task_manager.get_score("Student 1", "Task 2") << endl;
    cout << "Score for Student 1, Task 3: " << task_manager.get_score("Student 1", "Task 3") << endl;

    cout << "Overall result for Student 1: " << task_manager.calculate_result("Student 1") << endl;
    cout << "Overall result for Student 2: " << task_manager.calculate_result("Student 2") << endl;
    cout << "Overall result for Student 3: " << task_manager.calculate_result("Student 3") << endl;

    return 0;
}