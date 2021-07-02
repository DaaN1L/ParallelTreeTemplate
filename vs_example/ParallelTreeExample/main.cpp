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

#include <ParallelTree.hpp>

using namespace std;

struct Graph {
    vector <vector<int>> adjacencyList;
    vector<int> colors;
};

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
class ExampleRecord : public Record
{
public:
    ExampleRecord() :
        x(4, 0)
    {}
    
    // Вектор с решением
    vector<int> x;
    
    /*
     * Должна возвращать true, если данный рекорд лучше (меньше в задачах
     * минимизации и больше в задачах максимизации), чем other
     */
    bool betterThan(const Record& other) const override
    {
        const ExampleRecord& otherCast = static_cast<const ExampleRecord&>(other);
        // Поскольку у нас задача максимизации, то используем оператор "больше".
        return f(x) > f(otherCast.x);
    }
    
    // Должен возвращать копию данного рекорда.
    std::unique_ptr<Record> clone() const override
    {
        // Здесь просто используем конструктор копий
        return std::make_unique<ExampleRecord>(*this);
    }
};

// Узел дерева вариантов. Должен наследоваться от класса Node и реализовать
// методы process и hasHigherPriority.
class ExampleNode : public Node
{
public:
    ExampleNode() :
        x(4, 0),
        lastX(-1)
    {}
    
    // Вектор с 4 переменными x.
    vector<int> x;
    // Какой x мы меняли последним
    int lastX;
    
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
        ExampleRecord& recordCast = static_cast<ExampleRecord&>(record);
        
        // Потомки
        std::vector< std::unique_ptr<Node> > childNodes;
        // Если lastX == 3, то мы дошли до листа дерева и потомков у текущего
        // узла нет.
        if(lastX == 3)
        {
            // Если текущее решение лучше рекорда, то меняем рекорд
            if(f(x) > f(recordCast.x))
                recordCast.x = x;
            // Потомков нет. childNodes пуст.
            return childNodes;
        }
        else
        {
            lastX += 1;
            // Рассматриваем 2 случая: x[lastX] = 0 и x[lastX] = 1
            x[lastX] = 0;
            // Если ограничение не выполняется, то отсекаем ветвь.
            if(constraint(x))
                childNodes.emplace_back(new ExampleNode(*this));
            
            x[lastX] = 1;
            // Если ограничение не выполняется, то отсекаем ветвь.
            if(constraint(x))
                childNodes.emplace_back(new ExampleNode(*this));
            
            return childNodes;
        }
    }
    
    /*
     * Возвращает true, если приоритет данного задания больше, чем other.
     * Задания с большим приоритетом будут обрабатываться раньше.
     */
    bool hasHigherPriority(const Node& other) const override
    {
        const ExampleNode& otherCast = static_cast<const ExampleNode&>(other);
        // Если у данного узда значение f больше, то считаем что у него больше
        // приоритет.
        return f(x) > f(otherCast.x);
    }
};


int main()
{
    // Полагаем в начале все x равными 0 и начальный рекорд равным 0.
    ExampleRecord initialRecord;
    // Корень дерева вариантов.
    unique_ptr<ExampleNode> root = make_unique<ExampleNode>();
    // Параллельно находим решение
    unique_ptr<Record> bestSolution = parallelTree(move(root), initialRecord);
    const ExampleRecord* bestSolutionCast = reinterpret_cast<const ExampleRecord*>(bestSolution.get());
    
    cout << "x0 = " << bestSolutionCast->x[0] << ",  "
         << "x1 = " << bestSolutionCast->x[1] << ",  "
         << "x2 = " << bestSolutionCast->x[2] << ",  "
         << "x2 = " << bestSolutionCast->x[3] << ",  "
         << endl;
    
    return 0;
}
