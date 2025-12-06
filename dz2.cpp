#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>

struct Node {
    int data;
    Node* next;
    Node* prev;
    Node(int value) : data(value), next(nullptr), prev(nullptr) {}
};

class LinkedList {
private:
    Node* head;
    Node* tail;
    int type;
    bool isCircular() const { return type == 3 || type == 4; }
    bool isDoubly() const { return type == 2 || type == 4; }

public:
    LinkedList(int t) : head(nullptr), tail(nullptr), type(t) {}

    ~LinkedList() { clear(); }

    bool empty() const { return head == nullptr; }

    void add(int value) {
        Node* newNode = new Node(value);
        if (empty()) {
            head = tail = newNode;
            if (isCircular()) newNode->next = head;
            if (type == 4) newNode->prev = head;
        } else {
            switch (type) {
                case 1: tail->next = newNode; tail = newNode; break;
                case 2: newNode->prev = tail; tail->next = newNode; tail = newNode; break;
                case 3: newNode->next = head; tail->next = newNode; tail = newNode; break;
                case 4: newNode->next = head; newNode->prev = tail;
                    tail->next = newNode; head->prev = newNode; tail = newNode; break;
            }
        }
    }

    void print() const {
        if (empty()) { std::cout << "Empty\n"; return; }
        Node* curr = head;
        if (!isCircular()) {
            while (curr) {
                std::cout << curr->data;
                curr = curr->next;
                if (curr) std::cout << (isDoubly() ? " <-> " : " > ");
            }
        } else {
            do {
                std::cout << curr->data;
                curr = curr->next;
                if (curr != head) std::cout << (isDoubly() ? " <>0 " : " >0 ");
            } while (curr != head);
            std::cout << " (head)";
        }
        std::cout << "\n";
    }

    std::vector<int> find(int value) const {
        std::vector<int> positions;
        if (empty()) return positions;
        Node* curr = head;
        int pos = 0;
        if (!isCircular()) {
            while (curr) {
                if (curr->data == value) positions.push_back(pos);
                curr = curr->next;
                pos++;
            }
        } else {
            do {
                if (curr->data == value) positions.push_back(pos);
                curr = curr->next;
                pos++;
            } while (curr != head);
        }
        return positions;
    }

    bool remove(int value, bool all = false) {
        if (empty()) return false;
        bool removed = false;
        Node* curr = head;
        Node* prev = nullptr;

        while (curr) {
            if (curr->data == value) {
                Node* toDelete = curr;

                // Обновление связей
                if (curr == head) {
                    head = head->next;
                    if (head && isDoubly()) head->prev = nullptr;
                    if (isCircular()) tail->next = head;
                } else if (curr == tail) {
                    tail = prev;
                    if (tail) tail->next = (isCircular() ? head : nullptr);
                    if (isDoubly() && head) head->prev = tail;
                } else {
                    prev->next = curr->next;
                    if (isDoubly() && curr->next) curr->next->prev = prev;
                }

                curr = curr->next;
                delete toDelete;
                removed = true;

                if (!all) return true;
                if (!isCircular()) continue;
            } else {
                prev = curr;
                curr = curr->next;
            }

            // Проверка завершения для циклических списков
            if (isCircular() && curr == head) break;
        }
        return removed;
    }

    void clear() {
        if (empty()) return;
        if (isCircular()) tail->next = nullptr;
        Node* curr = head;
        while (curr) {
            Node* next = curr->next;
            delete curr;
            curr = next;
        }
        head = tail = nullptr;
    }

    bool save(const std::string& filename) const {
        std::ofstream file(filename);
        if (!file) return false;

        file << "type:" << (type == 1 ? ">" : type == 2 ? "<>" : type == 3 ? ">0" : "<>0") << "\n";
        file << "elements:";
        if (!empty()) {
            Node* curr = head;
            if (!isCircular()) {
                while (curr) {
                    file << curr->data;
                    curr = curr->next;
                    if (curr) file << ",";
                }
            } else {
                Node* start = head;
                do {
                    file << curr->data;
                    curr = curr->next;
                    if (curr != start) file << ",";
                } while (curr != start);
            }
        }
        file << "\n";
        return true;
    }

    bool load(const std::string& filename) {
        if (!empty()) { std::cout << "List exists\n"; return false; }
        std::ifstream file(filename);
        if (!file) { std::cout << "Can't open file\n"; return false; }

        std::string line, typeStr;
        std::vector<int> elements;

        if (std::getline(file, line) && line.find("type:") == 0) {
            typeStr = line.substr(5);
            if (typeStr == ">") type = 1;
            else if (typeStr == "<>") type = 2;
            else if (typeStr == ">0") type = 3;
            else if (typeStr == "<>0") type = 4;
            else return false;
        }

        if (std::getline(file, line) && line.find("elements:") == 0) {
            std::stringstream ss(line.substr(9));
            std::string token;
            while (std::getline(ss, token, ',')) {
                if (!token.empty()) elements.push_back(std::stoi(token));
            }
        }

        for (int val : elements) add(val);
        return true;
    }
};

bool parseArgs(int argc, char* argv[], int& t, int& c, std::vector<int>& e) {
    t = c = 0;
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "-t" && i+1 < argc) t = std::stoi(argv[++i]);
        else if (arg == "-c" && i+1 < argc) c = std::stoi(argv[++i]);
        else if (arg == "-e" && i+1 < argc) {
            std::stringstream ss(argv[++i]);
            std::string token;
            while (std::getline(ss, token, ',')) e.push_back(std::stoi(token));
        }
        else return false;
    }
    return t >= 1 && t <= 4 && c >= 0 && (int)e.size() == c;
}

void menu() {
    std::cout << "\nMenu:\n1. Add\n2. Print\n3. Find\n4. Delete first\n5. Delete all\n6. Clear\n7. Save\n8. Load\n0. Exit\nChoice: ";
}

int main(int argc, char* argv[]) {
    LinkedList* list = nullptr;

    // Парсинг аргументов командной строки
    if (argc > 1) {
        int t, c;
        std::vector<int> e;
        if (!parseArgs(argc, argv, t, c, e)) {
            std::cout << "Invalid args\n";
            return 1;
        }
        list = new LinkedList(t);
        for (int val : e) list->add(val);
        std::cout << "List created: type=" << t << ", count=" << c << "\n";
    } else {
        // Интерактивный режим
        int t, c;
        std::cout << "Type (1-4): ";
        std::cin >> t;
        if (t < 1 || t > 4) {
            std::cout << "Invalid type\n";
            return 1;
        }
        list = new LinkedList(t);
        std::cout << "Count: ";
        std::cin >> c;
        for (int i = 0; i < c; i++) {
            int val;
            std::cout << "Element " << i+1 << ": ";
            std::cin >> val;
            list->add(val);
        }
        std::cin.ignore();
    }

    // Главный цикл меню
    int choice;
    do {
        menu();
        std::cin >> choice;
        std::cin.ignore();

        switch (choice) {
            case 1: {
                int val;
                std::cout << "Value: ";
                std::cin >> val;
                list->add(val);
                break;
            }
            case 2:
                list->print();
                break;
            case 3: {
                int val;
                std::cout << "Value: ";
                std::cin >> val;
                auto pos = list->find(val);
                if (pos.empty()) std::cout << "Not found\n";
                else {
                    std::cout << "Positions: ";
                    for (size_t i = 0; i < pos.size(); i++)
                        std::cout << pos[i] << (i < pos.size()-1 ? ", " : "\n");
                }
                break;
            }
            case 4: {
                int val;
                std::cout << "Value: ";
                std::cin >> val;
                std::cout << (list->remove(val, false) ? "Deleted\n" : "Not found\n");
                break;
            }
            case 5: {
                int val;
                std::cout << "Value: ";
                std::cin >> val;
                std::cout << (list->remove(val, true) ? "All deleted\n" : "Not found\n");
                break;
            }
            case 6:
                list->clear();
                std::cout << "Cleared\n";
                break;
            case 7: {
                std::string fname;
                std::cout << "Filename: ";
                std::getline(std::cin, fname);
                std::cout << (list->save(fname) ? "Saved\n" : "Save failed\n");
                break;
            }
            case 8: {
                std::string fname;
                std::cout << "Filename: ";
                std::getline(std::cin, fname);
                std::cout << (list->load(fname) ? "Loaded\n" : "Load failed\n");
                break;
            }
            case 0:
                std::cout << "Exit\n";
                break;
            default:
                std::cout << "Invalid choice\n";
        }
    } while (choice != 0);

    delete list;
    return 0;
}