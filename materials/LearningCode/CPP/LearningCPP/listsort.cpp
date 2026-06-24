#include <filesystem>
#include <iostream>
#include <string>
#include <chrono>
#include <vector>

class Sort {
public:
    template<typename T>
    static void selection_sort(std::vector<T>& arr) {
        std::size_t n = arr.size();
        if (arr.empty()) {
            return;
        }

        for (int i = 0;i < n;i++) {
            int min_index = i;
            for (int j = i + 1;j < n;j++) {
                if (arr[j] < arr[min_index]) {
                    min_index = j;
                }
            }
            std::swap(arr[i], arr[min_index]);
        }
    }

    template<typename T>
    static void insertion_sort(std::vector<T>& arr) {
        std::size_t n = arr.size();
        if (arr.empty()) {
            return;
        }
        for (int i = 0;i < n;i++) {
            for (int j = i;j>0 && arr[j] < arr [j-1];j--) {
                std::swap(arr[j], arr[j-1]);
            }
        }
    }


    template<typename T>
    static void shell_sort(std::vector<T>& arr) {
        std::size_t n = arr.size();
        int h = 1;
        while (h < n/3) {
            h = 3 * h + 1;
        }
        while (h >= 1) {
            for (int i = h; i < n; i++) {
                for (int j = i; j >= h && arr[j]<arr[j-h]; j -= h) {
                    std::swap(arr[j], arr[j-h]);
                }
            }
            h = h / 3;
        }
    }
};

class Timer {
public:
    template<typename Func, typename... Args>
    static void measure_time(const std::string& label, Func func, Args&&... args) {
        std::cout << "--- 正在运行: " << label << " ---" << std::endl;
        auto start = std::chrono::high_resolution_clock::now();
        func(std::forward<Args>(args)...);
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> elapsed = end - start;
        std::cout << "耗时: " << elapsed.count() << " ms" << std::endl;
        std::cout << "-----------------------------------" << std::endl;
    }
};

class LinkList {
    public:
    // 节点结构体
    template<typename T>
    struct Listnode{
        T data;
        Listnode* next;
        Listnode(T x): data(x),next(nullptr){}
    };

    //寻找target的上一个节点
    template<typename T>
    static Listnode<T>* find_Prev(Listnode<T>* Head,Listnode<T>* target){
        if(Head == nullptr || target == Head){
            return nullptr;
        }
        Listnode<T>* temp = Head;
        while (temp != nullptr && temp->next != target){
            temp = temp->next;
        }
        return (temp->next == target) ? temp : nullptr;
    }

    // 将P节点插入到L后边
    template<typename T>
    static bool ListInsert(Listnode<T>* L,Listnode<T>* P) {
      Listnode<T> *n1 = L->next;
      L->next = P;
      P->next = n1;
      return true;
    }

    // 删除节点
    template<typename T>
    static bool Remove(Listnode<T>* &Head,Listnode<T>* &P){
        if(P == nullptr || Head == nullptr){
            return false;
        }
        if (P == Head){
            Head = Head->next;
            delete P;
            P = nullptr;
            return true;
        }
        else {
            Listnode<T>* prev = find_Prev(Head,P);
            if (prev == nullptr){
                return false;
            }
            prev->next = P->next;
            delete P;
            P = nullptr;
            return true;
        }
    }
};





int main() {
    std::vector<std::string> arr= {"cherry", "apple", "banana", "Apple"};
    Sort::shell_sort<std::string>(arr);
    std::cout << "Sorted array: \n";
    for (const auto &x : arr) {
        std::cout << x << " ";
    }
    std::cout << std::endl;
    Timer::measure_time("Shell Sort", Sort::shell_sort<std::string>, arr);
    return 0;
}