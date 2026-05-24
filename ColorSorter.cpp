#include <algorithm>
#include <bitset>
#include <cstdint>
#include <cstring>
#include <initializer_list>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <utility>

using namespace std;

uint64_t hashVec(const vector<uint32_t> &vec) {
    uint64_t hash = 14695981039346656037ULL;
    uint64_t prime = 1099511628211ULL;

    const uint8_t *bytes = reinterpret_cast<const uint8_t *>(vec.data());
    size_t total_bytes = vec.size() * sizeof(uint32_t);

    for (size_t i = 0; i < total_bytes; ++i) {
        hash ^= bytes[i];
        hash *= prime;
    }

    return hash;
}

struct Tube {
    char data[5];
    size_t size;

    Tube() : data{0, 0, 0, 0, 0}, size{0} {}

    Tube(const char input[5], size_t size) : size{size} {
        for (size_t i = 0; i < size && i < 4; ++i) {
            data[i] = input[i];
        }
        for (size_t i = size; i < 5; ++i) {
            data[i] = 0;
        }
    }

    void push(char color) {
        if (color == 0) {
            throw invalid_argument("Zero cannot be a color.");
        }

        if (size < 4) {
            data[size] = color;
            ++size;
            data[size] = 0;
        } else {
            throw out_of_range("Cannot add to a full tube!");
        }
    }

    char pop() {
        if (size > 0) {
            --size;
            char top = data[size];
            data[size] = 0;

            return top;
        } else {
            throw out_of_range("Cannot pop an empty tube!");
        }
    }

    char top() {
        if (size == 0) {
            return -1;
        } else {
            return data[size - 1];
        }
    }

    bool isAllSame() const {
        char first = 0;

        for (int i = 0; i < 4; ++i) {
            if (data[i] == 0) {
                break;
            }

            if (first == 0) {
                first = data[i];
            } else if (data[i] != first) {
                return false;
            }
        }

        return first != 0;
    }

    bool isComplete() const {
        for (int i = 0; i < size; ++i) {
            if (data[0] != data[i]) {
                return false;
            }
        }

        return size == 0 || size == 4;
    }

    bool isFull() const {
        return size == 4;
    }

    bool isEmpty() const {
        return size == 0;
    }

    uint32_t toInt() const {
        uint32_t out = 0;

        for (int i = 0; i < size; ++i) {
            out += data[i] << (8 * i);
        }

        return out;
    }
};

typedef pair<int, int> Move;

class WaterPuzzle {
   public:
    vector<Tube> thePuzzle;

    bool isComplete() {
        for (const Tube &tube : thePuzzle) {
            if (!tube.isComplete()) {
                return false;
            }
        }

        return true;
    }

    /**
     * Pours from tubeA INTO tubeB.
     */
    int pour(size_t tubeA, size_t tubeB) {
        if (tubeA >= thePuzzle.size()) {
            throw out_of_range("TubeA out of range");
        }

        if (tubeB >= thePuzzle.size()) {
            throw out_of_range("TubeB out of range");
        }

        if (!thePuzzle[tubeB].isEmpty() && thePuzzle[tubeB].top() != thePuzzle[tubeA].top()) {
            throw invalid_argument("Cannot pour - top of tubeB is not equal to top of tubeA.");
        }

        char topColor = thePuzzle[tubeA].top();
        int numPoured = 0;

        while (thePuzzle[tubeA].top() == topColor && !thePuzzle[tubeB].isFull()) {
            char top = thePuzzle[tubeA].pop();
            thePuzzle[tubeB].push(top);
            ++numPoured;
        }

        return numPoured;
    }

    void unpour(size_t tubeB, size_t tubeA, int numPoured) {
        if (tubeA >= thePuzzle.size()) {
            throw out_of_range("TubeA out of range");
        }

        if (tubeB >= thePuzzle.size()) {
            throw out_of_range("TubeB out of range");
        }

        if (thePuzzle[tubeB].size < numPoured && thePuzzle[tubeA].size + numPoured > 4 &&
            !(thePuzzle[tubeB].size == numPoured)) {
            throw invalid_argument("Cannot unpour, tubeB and tubeA do not satisfy the requirements.");
        }

        char topColor = thePuzzle[tubeB].top();

        for (int i = 0; i < numPoured; ++i) {
            int index = thePuzzle[tubeB].size - 1 - i;

            if (thePuzzle[tubeB].data[index] != topColor) {
                throw invalid_argument("Cannot unpour, top n colors of tubeB are not the same.");
            }
        }

        if (numPoured < thePuzzle[tubeB].size) {
            int index = thePuzzle[tubeB].size - 1 - numPoured;

            if (thePuzzle[tubeB].data[index] != topColor) {
                throw invalid_argument("Cannot unpour, top n+1 colors of tubeB are not the same.");
            }
        }

        thePuzzle[tubeA].push(thePuzzle[tubeB].pop());
    }

    vector<Move> moveGen() {
        vector<Move> theMoves;

        for (int i = 0; i < thePuzzle.size(); ++i) {
            for (int j = 0; j < thePuzzle.size(); ++j) {
                if (i != j && !thePuzzle[j].isFull() && !thePuzzle[i].isEmpty() &&
                    (thePuzzle[j].isEmpty() || thePuzzle[j].top() == thePuzzle[i].top())) {
                    theMoves.push_back({i, j});
                }
            }
        }

        return theMoves;
    }

    uint64_t compress() const {
        vector<uint32_t> theList;

        for (const Tube &tube : thePuzzle) {
            theList.push_back(tube.toInt());
        }

        return hashVec(theList);
    }

    bool readPuzzle() {
        thePuzzle.clear();
        string line;

        while (true) {
            if (!std::getline(cin, line)) {
                return false;
            }

            if (line == ".END") {
                return true;
            }

            if (line.empty()) {
                continue;
            }

            if (line == ".EMPTY") {
                thePuzzle.emplace_back("", 0);
                continue;
            }

            if (line.size() > 4) {
                throw invalid_argument("Tube description must have at most 4 characters.");
            }

            Tube tube;
            for (char c : line) {
                tube.push(c);
            }
            thePuzzle.push_back(tube);
        }
    }

    void print() {
        const int HEIGHT = 4;
        size_t n = thePuzzle.size();

        for (int level = HEIGHT - 1; level >= 0; --level) {
            for (size_t t = 0; t < n; ++t) {
                if (thePuzzle[t].size > static_cast<size_t>(level)) {
                    cout << ' ' << thePuzzle[t].data[level] << ' ';
                } else {
                    cout << "   ";
                }

                if (t + 1 < n) cout << ' ';
            }

            cout << '\n';
        }

        for (size_t t = 0; t < n; ++t) {
            cout << "---";
            if (t + 1 < n) cout << ' ';
        }

        cout << '\n';

        for (size_t t = 0; t < n; ++t) {
            cout << ' ' << t << ' ';
            if (t + 1 < n) cout << ' ';
        }

        cout << '\n'
             << "--- END OF PUZZLE ---" << endl;
    }
};

void printMoves(const vector<Move> &moves) {
    cout << "--- Begin Move List ---" << endl;
    for (const Move &move : moves) {
        cout << move.first << " -> " << move.second << endl;
    }
}

bool solveRecursive(vector<Move> &moveList, unordered_set<uint64_t> &visited, WaterPuzzle &state) {
    if (state.isComplete()) {
        return true;
    }

    uint64_t hashed = state.compress();

    if (visited.count(hashed) > 0) {
        return false;
    }

    visited.insert(hashed);
    vector<Move> moves = state.moveGen();

    for (const Move &move : moves) {
        if (state.thePuzzle[move.first].isAllSame() && state.thePuzzle[move.second].isEmpty()) {
            continue;
        }

        WaterPuzzle nextState = state;

        int numPoured = nextState.pour(move.first, move.second);
        moveList.push_back(move);

        // nextState.print();

        if (solveRecursive(moveList, visited, nextState)) {
            return true;
        }

        nextState.unpour(move.second, move.first, numPoured);
        moveList.pop_back();
    }

    return false;
}

vector<Move> solve(WaterPuzzle state) {
    vector<Move> moveList{};
    unordered_set<uint64_t> visited{};

    solveRecursive(moveList, visited, state);

    return moveList;
}

int main(int argc, char *argv[]) {
    WaterPuzzle puzzle{};

    puzzle.readPuzzle();

    vector<Move> solution = solve(puzzle);


    if (argc > 1 && strcmp(argv[1], "verbose") == 0) {
        cout << "Sol len.: " << solution.size() << endl;
    }

    int i = 1;
    for (const Move &move : solution) {
        if (argc > 1 && strcmp(argv[1], "verbose") == 0) {
            cout << i++ << ". " << move.first << " -> " << move.second << endl;
        } else {
            cout << move.first << " " << move.second << endl;
        }

        puzzle.pour(move.first, move.second);
        // puzzle.print();
    }
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started:
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
