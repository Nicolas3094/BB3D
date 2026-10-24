#include "../src/readData/ReadData.hpp"
#include "../src/readData/ExperimentResults.hpp"
#include "../src/genetic/GenomeOperators.hpp"
#include "../src/metaheuristics/GeneticAlgorithm.hpp"
#include "../src/structures/PriorityPointQueue.hpp"
#include <utility>
#include <cstdio>
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

void require(bool value, const char *message) { if (!value) throw std::runtime_error(message); }
template<typename Action> void rejects(Action action)
{
    bool caught = false; try { action(); } catch (const std::exception &) { caught = true; }
    require(caught, "Invalid input did not produce an error");
}
std::string fixture(const std::string &name, const std::string &contents)
{
    const std::string path = "build/infrastructure-tests/" + name;
    std::ofstream output(path); output << contents; output.close();
    require(!output.fail(), "Cannot create test fixture"); return path;
}
ItemBin box(int id) { return ItemBin::Build().setId(id).setTipo(id).setLargo(1).setAncho(1).setAlto(1).setRotations(Punto::Build(1,1,1)); }
void checkReading()
{
    require(getNumbersFromLines("  1\t 2   \r") == std::vector<int>({1,2}), "Whitespace parsing failed");
    require(getNumbersFromLines("  \t\r").empty(), "Blank row should be empty");
    rejects([] { getNumbersFromLines("1 2x"); });
    rejects([] { getNumbersFromLines("2147483648"); });
    auto path = fixture("valid.txt", "1 2\n\n1 42\n4 4 4\n1\n1 2 1 1 1 1 1 1\n");
    auto data = readData(path);
    require(data.size() == 1 && data[0].totalItems.size() == 2 && data[0].totalItems[1].getId() == 2, "Valid dataset parsed incorrectly");
    for (const std::string &invalid : {
        std::string(""), std::string("1 2\n"),
        std::string("1 2\n1 42\n4 4 4\n2\n1 2 1 1 1 1 1 1\n"),
        std::string("1 2\n1 42\n4 4 4\n1\n1 3 1 1 1 1 1 1\n"),
        std::string("1 2\n1 42\n4 4 4\n1\n1 1 1 1 1 1 1 1\n"),
        std::string("1 2\n1 42\n4 0 4\n1\n1 2 1 1 1 1 1 1\n"),
        std::string("1 2\n1 42\n4 4 4\n1\n1 2 2 1 1 1 1 1\n"),
        std::string("1 2\n1 42\n4 4 4\n1\n1 2 1 1 1 1 1 1\n9\n")})
        rejects([&] { readData(fixture("invalid.txt", invalid)); });
    std::remove("build/infrastructure-tests/missing.txt");
    rejects([] { readData("build/infrastructure-tests/missing.txt"); });
    std::string manyFitness, manyTimes;
    for (int i = 0; i < 150; ++i) { manyFitness += "0.5\n"; manyTimes += "5000000000\n"; }
    auto fitness = getFloatNumberListFile(fixture("fitness.txt", manyFitness));
    auto times = getIntegerNumberListFile(fixture("times.txt", manyTimes));
    require(fitness.size() == 150 && times.size() == 150 && times[149] == 5000000000LL, "Result reader truncated rows or times");
    require(getFloatNumberListFile("build/infrastructure-tests/missing.txt").empty(), "Missing results should be empty");
    rejects([&] { getFloatNumberListFile(fixture("bad-result.txt", "0.5 junk\n")); });
    for (auto name : {"P1A2", "P2A2", "P3A2", "P4A2", "P5A2"})
    {
        auto instances = readData(std::string("Instance/") + name + ".csv");
        require(instances.size() == 100, "A real dataset lost instances");
        for (const auto &instance : instances) require(instance.totalItems.size() == instances[0].totalItems.size(), "Dataset item counts differ");
    }
    std::cout << "PASS: whitespace, malformed/missing datasets, 150 result rows, int64 times and P1-P5\n";
}
void checkResults()
{
    require(!isExperimentComplete(ExperimentResult{-1,-1,ExperimentStatus::Running}), "Interrupted experiment was considered complete");
    require(isExperimentComplete(ExperimentResult{0,0,ExperimentStatus::Complete}), "Zero fitness/time completion was lost");
    const std::string fitness = "build/infrastructure-tests/saved-fitness.csv";
    const std::string times = "build/infrastructure-tests/saved-time.csv";
    std::remove((fitness + ".checkpoint").c_str());
    std::remove(fitness.c_str()); std::remove(times.c_str());
    auto missing = loadExperimentResults(fitness, times, 3);
    require(missing.size() == 3 && missing[0].status == ExperimentStatus::Pending, "Missing results not initialized");
    fixture("saved-fitness.csv", "-1\n0.5\n0\n");
    fixture("saved-time.csv", "-1\n0\n0\n");
    auto legacy = loadExperimentResults(fitness, times, 3);
    require(legacy[0].status == ExperimentStatus::Running && isExperimentComplete(legacy[1]) &&
        legacy[2].status == ExperimentStatus::Pending, "Legacy recovery lost completion state");
    ExperimentResults expected(150, ExperimentResult{0.12345678901234567,5000000000LL,ExperimentStatus::Complete});
    expected[0] = {0,0,ExperimentStatus::Complete};
    expected[1] = {-1,-1,ExperimentStatus::Running}; expected[2] = {};
    saveExperimentResults(expected, fitness, times);
    auto loaded = loadExperimentResults(fitness, times, expected.size());
    for (size_t i = 0; i < expected.size(); ++i)
        require(loaded[i].fitness == expected[i].fitness && loaded[i].milliseconds == expected[i].milliseconds &&
            loaded[i].status == expected[i].status, "Checkpoint roundtrip changed values");
    require(getFloatNumberListFile(fitness).size() == 150 && getIntegerNumberListFile(times).size() == 150, "CSV exports lost rows");
#ifdef _WIN32
    HANDLE locked = CreateFileA((fitness + ".checkpoint").c_str(), GENERIC_READ, FILE_SHARE_READ,
        nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    require(locked != INVALID_HANDLE_VALUE, "Cannot lock checkpoint fixture");
    bool failed = false;
    try { saveExperimentResults({{0.9,1,ExperimentStatus::Complete}},fitness,times); }
    catch (const std::exception &) { failed = true; }
    CloseHandle(locked);
    require(failed && loadExperimentResults(fitness,times,150)[149].fitness == expected[149].fitness,
        "Failed atomic replacement damaged the previous checkpoint");
#endif
    fixture("saved-fitness.csv", "corrupt export\n"); fixture("saved-time.csv", "-1\n");
    require(isExperimentComplete(loadExperimentResults(fitness,times,150)[0]), "Recovery used inconsistent CSV exports");
    rejects([&] { loadExperimentResults(fitness,times,149); });
    rejects([&] { saveExperimentResults({{1.5,0,ExperimentStatus::Complete}},fitness,times); });
    require(loadExperimentResults(fitness,times,150).size() == 150, "Invalid save replaced valid checkpoint");
    fixture("saved-fitness.csv.checkpoint", "BB3D_RESULTS_V1\n2,,0,0\n");
    rejects([&] { loadExperimentResults(fitness,times,1); });
    fixture("saved-fitness.csv.checkpoint", "BB3D_RESULTS_V1\n");
    rejects([&] { loadExperimentResults(fitness,times,150); });
    fixture("not-a-directory", "block directory creation");
    const std::string blocked = "build/infrastructure-tests/not-a-directory/time.csv";
    rejects([&] { saveExperimentResults(expected,fitness,blocked); });
    require(loadExperimentResults(fitness,blocked,150)[149].milliseconds == 5000000000LL,
        "Partial CSV export damaged authoritative checkpoint");
    saveExperimentResults(expected,fitness,times);
    std::cout << "PASS: checkpoint roundtrip, zero results, retries, legacy import, corruption and export failure recovery\n";
}
void verifyLinks(PriorityPointQueue &queue)
{
    int count = 0; Node<Punto> *previous = nullptr;
    for (auto *node = queue.getRear(); node; node = node->next)
    {
        require(node->previous == previous, "Queue has a stale previous link");
        previous = node; require(++count < 1000, "Queue contains a cycle");
    }
    require(count == queue.getSize(), "Queue size disagrees with its nodes");
}
void checkQueue()
{
    auto queue = PriorityPointQueue::Build();
    queue.enqueue(Punto::Build(0,0,0)); queue.enqueue(Punto::Build(1,0,0)); queue.pop();
    require(queue.getSize() == 1 && queue.getRear()->previous == nullptr, "pop left a stale size/link");
    queue.pop(); queue.pop(); verifyLinks(queue);
    queue.enqueue(Punto::Build(0,0,0)); queue.enqueue(Punto::Build(1,0,0)); queue.enqueue(Punto::Build(2,0,0));
    PriorityPointQueue copy(queue);
    require(copy.getRear() != queue.getRear(), "Queue copy shares node ownership");
    copy.pop(); require(queue.getSize() == 3, "Copy mutation changed original");
    PriorityPointQueue assigned; assigned = queue; assigned = assigned;
    queue.clear(); require(assigned.getSize() == 3, "Copy assignment lost nodes");
    PriorityPointQueue moved(std::move(assigned));
    require(assigned.isEmpty() && assigned.getSize() == 0, "Move constructor retained source ownership");
    PriorityPointQueue target; target.enqueue(Punto::Build(9,0,0)); target = std::move(moved);
    require(moved.isEmpty() && target.getSize() == 3, "Move assignment failed");
    target.removeNode(target.getRear()->next); verifyLinks(target);
    target.removeNode(target.getRear()->next); verifyLinks(target);
    target.removeNode(target.getRear()); target.removeNode(nullptr); verifyLinks(target);
    verifyLinks(copy);
    std::cout << "PASS: queue pop, independent copies, assignments, moves and node removal\n";
}
void checkGenomes()
{
    LoadedBins items{box(1),box(2),box(3)};
    require(decodeChromosome({1}, items)[0].getId() == 1, "Decoder uses zero-based IDs");
    auto decoded = decodeChromosome({3,1}, items);
    require(decoded[0].getId() == 3 && decoded[1].getId() == 1, "Subset decoding order changed");
    for (Chromosome invalid : {Chromosome{0}, Chromosome{-1}, Chromosome{4}, Chromosome{1,1}})
        rejects([&] { decodeChromosome(invalid, items); });
    for (int count : {0,1,2,3,4,7})
    {
        auto population = buildCompleteHeuristicChromosomes(items, count);
        require(population.size() == static_cast<size_t>(count), "Population count changed");
        for (auto ids : population) { std::sort(ids.begin(), ids.end()); require(ids == Chromosome({1,2,3}), "Population has invalid IDs"); }
    }
    auto arbitrary = buildChromosomes({box(10),box(20)}, 3);
    for (auto ids : arbitrary) { std::sort(ids.begin(), ids.end()); require(ids == Chromosome({10,20}), "Generator invented IDs"); }
    rejects([&] { buildCompleteHeuristicChromosomes(items, -1); });
    rejects([&] { buildChromosomes(items, -1); });
    rejects([] { generatePermutation(-1); });
    rejects([] { generateRandomRepeatedAlalleleChromosome(-1,1,6); });
    rejects([] { generateRandomRepeatedAlalleleChromosome(3,3,2); });
    std::vector<int> empty; rejects([&] { getNum(empty); });
    require(generateRandomRepeatedAlalleleChromosome(3,1,0) == Chromosome({0,0,0}), "Fixed rotations changed");
    std::cout << "PASS: one-based decoding, subsets, population sizes 0-7, original IDs and invalid sizes\n";
}
int main(int argc, char **argv)
{
    try
    {
        const int group = argc > 1 ? std::stoi(argv[1]) : 0;
        if (group == 0 || group == 9) checkReading();
        if (group == 0 || group == 10) checkResults();
        if (group == 0 || group == 11) checkQueue();
        if (group == 0 || group == 12) checkGenomes();
        return 0;
    }
    catch (const std::exception &error) { std::cerr << "FAIL: " << error.what() << '\n'; return 1; }
}
