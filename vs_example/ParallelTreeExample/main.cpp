/*
 * Дана задача: Раскраска графа (переборный алгоритм)
 * Написать программу, которая для заданного графа G(X,U) строит правильную раскраску вершин
 * этого графа в минимальное возможное количество цветов. Раскраска считается допустимой,
 * если концы любого ребра графа раскрашены в разные цвета.
 * Для раскраски использовать метод полного перебора возможных отображений вершин во множество цветов.
 */

#include <iostream>
#include <memory>
#include <vector>
#include <set>

#include <ParallelTree.hpp>

using namespace std;

struct Graph {
    vector <vector<int>> adjacencyList;
    vector<int> colors;

    Graph(vector< vector<int> > v) : adjacencyList(v), colors(v.size(), -1) {}
};

// Функция, которая считает количество различных цветов 
int count_unique_elem(vector<int> arr) {
    return set<int>(arr.begin(), arr.end()).size();
}

// Функция, которая проверяет, правильно ли раскрашен граф
bool is_correctly_colored(Graph g) {

    // Проходим по каждой вершине графа
    for (int i = 0; i < g.adjacencyList.size(); ++i) {

        // Вершина должна иметь цвет
        if (g.colors[i] == -1)
            continue;

        // Проходим по соседям данной вершины. Если цвет какого-либо соседа совпадает, граф раскрашен неправильно
        for (int neighbor : g.adjacencyList[i]) {
            if (g.colors[i] == g.colors[neighbor])
                return false;
        }
    }

    return true;
}

// Рекорд. Должен наследоваться от класса Record и реализовать методы
// betterThan и clone.
class MyRecord : public Record
{
public:
    // Вектор с решением
    vector<int> colors;

    MyRecord(int n) : colors(n) {
        for (int i = 0; i < n; ++i)
            colors[i] = i;
    }

    /*
     * Должна возвращать true, если данный рекорд лучше (меньше в задачах
     * минимизации и больше в задачах максимизации), чем other
     */
    bool betterThan(const Record& other) const override
    {
        const MyRecord& otherCast = static_cast<const MyRecord&>(other);
        return count_unique_elem(colors) < count_unique_elem(otherCast.colors);
    }

    // Должен возвращать копию данного рекорда.
    std::unique_ptr<Record> clone() const override
    {
        // Здесь просто используем конструктор копий
        return std::make_unique<MyRecord>(*this);
    }
};

// Узел дерева вариантов. Должен наследоваться от класса Node и реализовать
// методы process и hasHigherPriority.
class MyNode : public Node
{
public:
    Graph g;
    // Какой узел мы меняли последним
    int lastNode;

    MyNode(const Graph& g) : lastNode(-1), g(g) {}

    /*
     * Функция, которая обрабатывает текущий узел и возвращает вектор
     * потомков этого узла (или пустой вектор, если потомков нет).
     *
     * Она не должна менять глобальных переменных, т.к. она будет исполняться
     * в нескольких потоках. Рекорд менять можно (при этом синхронизация не
     * требуется).
     */
    std::vector< std::unique_ptr<Node> > process(Record& record) override
    {
        MyRecord& recordCast = static_cast<MyRecord&>(record);

        // Потомки
        std::vector< std::unique_ptr<Node> > childNodes;

        // Если lastNode == n - 1, то мы дошли до листа дерева и потомков у текущего
        // узла нет.
        if (lastNode == g.adjacencyList.size() - 1) {
            // Если текущее решение лучше рекорда, то меняем рекорд
            if (count_unique_elem(g.colors) < count_unique_elem(recordCast.colors))
                recordCast.colors = g.colors;

            // Потомков нет. childNodes пуст.
            return childNodes;
        }
        else {
            lastNode += 1;

            // Рассматриваем n случаев (для каждого цвета)
            for (int i = 0; i < g.adjacencyList.size(); ++i) {
                g.colors[lastNode] = i;

                // Если раскраска неправильная, то отсекаем ветвь
                if (is_correctly_colored(g))
                    childNodes.emplace_back(new MyNode(*this));
            }

            return childNodes;
        }
    }

    /*
     * Возвращает true, если приоритет данного задания больше, чем other.
     * Задания с большим приоритетом будут обрабатываться раньше.
     */
    bool hasHigherPriority(const Node& other) const override {
        const MyNode& otherCast = static_cast<const MyNode&>(other);
        // Если у данного узла меньше уникальных цветов, 
        // то считаем что у него больше приоритет.
        return count_unique_elem(g.colors) < count_unique_elem(otherCast.g.colors);
    }
};


int main() {
    vector <vector<int>> v = { {1,4}, {0, 2, 3, 4}, {1,3, 4}, {1, 2}, {0, 1, 2} };
    Graph g(v);

    // Вначале каждй узел раскрашен в разный цвет 
    MyRecord initialRecord(v.size());

    // Корень дерева вариантов.
    unique_ptr<MyNode> root = make_unique<MyNode>(g);

    // Параллельно находим решение
    unique_ptr<Record> bestSolution = parallelTree(move(root), initialRecord);
    const MyRecord* bestSolutionCast = reinterpret_cast<const MyRecord*>(bestSolution.get());

    cout << "Correct graph coloring: ";
    for (int i = 0; i < v.size(); ++i)
        cout << bestSolutionCast->colors[i] << " ";
    cout << "\nChromatic number of a graph: " << count_unique_elem(bestSolutionCast->colors) << endl;

    return 0;
}
