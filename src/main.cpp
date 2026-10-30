#include "main.hpp"
#include "BRExperimentMenu.hpp"
#include "BRExperimentSession.hpp"
#include "brkga/BRKGARunner.hpp"

#include <chrono>
#include "genetic/FitnessOperators.hpp"
#include "genetic/GeneticsOperators.hpp"
#include "metaheuristics/GeneticAlgorithm.hpp"
#include "metaheuristics/ArtificialBeeColonyAlgorithm.hpp"
#include "metaheuristics/FireflyAlgorithm.hpp"
#include "structures/PriorityPointQueue.hpp"
#include "structures/ColaPrioridad.hpp"
#include "readData/Env.hpp"
#include "readData/ExperimentResults.hpp"
#include "readData/ExperimentConfiguration.hpp"
#include "readData/CommandLine.hpp"
#include "readData/ExperimentRunner.hpp"
#include "util/RandomNumbers.hpp"
#ifdef _WIN32
#include <io.h>
#include <cstdio>
#endif

namespace
{
struct KeepInteractiveConsole
{
    bool enabled = false;

    ~KeepInteractiveConsole()
    {
#ifdef _WIN32
        if (enabled && _isatty(_fileno(stdin)))
        {
            std::cout << "\nPresiona Enter para cerrar..." << std::flush;
            std::string ignored;
            std::getline(std::cin, ignored);
        }
#endif
    }
};
}

std::string LOCAL_PATH;
std::uint64_t configuredSeed = 20261024;
DBLFCandidatePolicy configuredCandidates = DBLFCandidatePolicy::ThreePoints;

int main(int argc, char **argv)
{
    KeepInteractiveConsole console;
    console.enabled = argc == 1;

    for (int i = 1; i < argc; ++i)
        if (std::string(argv[i]) == "--menu")
            console.enabled = true;
    try
    {
        auto options = parseCommandLine(std::vector<std::string>(argv + 1, argv + argc));

        if (options.help)
        {
            std::cout << commandLineHelp();

            return 0;
        }
        if (options.root.empty())
        {
            std::ifstream local("Instance/BR/BR0.txt");
            options.root = options.interactive && local.good() ? "." : getEnv("LOCAL_PATH");
        }
        LOCAL_PATH = options.root;
        configuredSeed = options.seed;
        configuredCandidates = options.candidates;

        if (options.interactive)
        {
            for (;;)
            {
                const auto selection = BRExperimentMenu().choose(std::cin, std::cout, options);

                if (!selection.confirmed)
                    return 0;

                const auto directory = BRExperimentSession().execute(selection);
                std::cout << "\nResultados guardados en: " << directory << "\nInforme: " << directory
                          << "/resumen.md\nCSV: " << directory << "/resultados.csv\n";
                for (;;)
                {
                    std::cout << "Otra sesion BR? 1=si, 0=salir [0]: " << std::flush;
                    std::string answer;

                    if (!std::getline(std::cin, answer))
                        return 0;

                    if (answer == "1")
                        break;

                    if (answer.empty() || answer == "0" || answer == "salir")
                        return 0;

                    std::cout << "Seleccion invalida.\n";
                }
            }
        }
        if (options.algorithm == "BRKGA2012")
        {
            if (options.sessionReport)
                throw std::invalid_argument(
                    "BRKGA usa sus propios informes; --report corresponde a GA/ABC/FFA.");
            BRMenuSelection selection;
            selection.options = options;
            selection.confirmed = true;
            selection.sessionName = "brkga2012";
            const auto directory = brkga2012::executeMenu(selection);
            std::cout << "Resultados BRKGA guardados en: " << directory << '\n';

            return 0;
        }
        if (options.sessionReport)
        {
            const auto directory = BRExperimentSession().resume(options);
            std::cout << "Informe BR guardado en: " << directory << "/resumen.md\n";
        }
        else
            runExperiments(options);

        return 0;
    }
    catch (const std::exception &error)
    {
        std::cerr << error.what() << '\n';

        return 1;
    }
}

void testInverseMutation()
{
    DoubleGenome zerosChromosome;
    flipMutation(zerosChromosome, 0.01, ROTATION_WAY::SIX_WAY);
    std::cout << zerosChromosome << "\n";
}

void iterGeneticAll()
{

    string rotationType, algorithmOption, problem, mut, name, isReplace, generalPath, fitnessDataPath,
        timePATH;
    bool isWithReplacement;

    // std::cout << "Select algorithm: [1] Genetic Algorithm [2] Artificial Bee Colony Algorithm [3] Firefly Algorithm\n\n";
    // std::cin >> algorithmOption;

    // std::cout << "Select rotation type:\n[1] ZERO-ROTATION\n[2] 2-ROTATION\n[3] 6-ROTATION\n";
    // std::cin >> rotationType;

    // std::cout << "Select problem:\n[1] P4A2 \n[2] P5A2\n\n";
    // std::cin >> problem;

    // std::cout << "Select mutation: [1] Inverse Mutation [2] Group 1 [3] Group 2\n\n";
    // std::cin >> mut;

    // std::cout << "With Replace: [1] Yes [2] No\n\n";
    // std::cin >> isReplace;

    handleUserInputs(
        /* algorithmOption= */ algorithmOption,
        /* rotationType= */ rotationType,
        /* algorithnName= */ name,
        /* replace= */ isReplace,
        /* isWithReplacement= */ isWithReplacement);

    vector<bool> replace{true, false};
    vector<string> ALGORITHMS{getAlgorithmNameFromNumber("1"), getAlgorithmNameFromNumber("2"),
                              getAlgorithmNameFromNumber("3")};
    vector<string> PROBLEMS{"P1A2", "P2A2", "P3A2", "P4A2", "P5A2"};
    vector<MutationType> MUTATIONS{getMutationTypeFromUser("1"), getMutationTypeFromUser("2"),
                                   getMutationTypeFromUser("3")};
    vector<ROTATION_WAY> ROTATIONS{ROTATION_WAY::ZERO_WAY, ROTATION_WAY::TWO_WAY, ROTATION_WAY::SIX_WAY};

    for (bool isWithReplacement : replace)
    {
        for (string algorithm : ALGORITHMS)
        {
            cout << "\nMetaheuristics: " << algorithm << "\n";

            for (ROTATION_WAY rotation : ROTATIONS)
            {
                cout << "\nRotation: " << rotation << "\n";

                for (auto problemName : PROBLEMS)
                {
                    vector<DatasetBinBacking> DATASSET =
                        readData(LOCAL_PATH + "/Instance/" + problemName + ".csv");
                    const std::string datasetHash =
                        datasetFingerprint(LOCAL_PATH + "/Instance/" + problemName + ".csv");

                    for (MutationType mutationTyped : MUTATIONS)
                    {
                        cout << "\n\nAlgorithm: " << problemName << " - " << mutationTyped << "\n";

                        handlePaths(isWithReplacement,
                                    /* algorithm= */ algorithm,
                                    /*problemName= */ problemName, mutationTyped, rotation,
                                    /* fitnessDataPath= */ fitnessDataPath,
                                    /* timePATH= */ timePATH);
                        auto config = makeExperimentConfiguration(algorithm, rotation, mutationTyped,
                                                                  isWithReplacement);
                        config.candidates = configuredCandidates;
                        const std::string identity = config.identity(datasetHash, configuredSeed);
                        const std::string suffix = "." + experimentFingerprint(identity) + ".csv";
                        fitnessDataPath += suffix;
                        timePATH += suffix;
                        ExperimentResults results =
                            loadExperimentResults(fitnessDataPath, timePATH, DATASSET.size(), identity);

                        for (int i = 0; i < DATASSET.size(); i++)
                        {

                            if (isExperimentComplete(results[i]))
                            {
                                continue;
                            }

                            cout << (i + 1) << "|";

                            Poblacion bestPob;

                            // Set -1 to inform it is being attended
                            results[i] = {-1, -1, ExperimentStatus::Running};

                            // Print it
                            saveExperimentResults(results, fitnessDataPath, timePATH, identity);

                            auto start = chrono::steady_clock::now();
                            seedRandomNumbers(
                                instanceSeed(configuredSeed, DATASSET[i].id, DATASSET[i].PSeed));
                            bestPob = config.run(DATASSET[i]);
                            auto stop = chrono::steady_clock::now();
                            auto duration = chrono::duration_cast<chrono::milliseconds>(stop - start);

                            // update data
                            results =
                                loadExperimentResults(fitnessDataPath, timePATH, DATASSET.size(), identity);
                            if (bestPob.empty())
                                throw std::runtime_error("Algorithm returned an empty population.");

                            results[i] = {bestPob[0].getFitness(), duration.count(),
                                          ExperimentStatus::Complete};
                            saveExperimentResults(results, fitnessDataPath, timePATH, identity);
                        }
                    }
                }
            }
        }
    }
}

void handleUserInputs(string &algorithmOption, string &rotationType, string algorithnName, string replace,
                      bool &isWithReplacement)
{
    isWithReplacement = replace == "1";

    /*if (algorithmOption != "1" && algorithmOption != "2" && algorithmOption != "3")
  {
    cout << "\nSelect valid algorithm.\n";

    return iterGeneticAll();
  }
  if (rotationType != "1" && rotationType != "2" && rotationType != "3")
  {
    cout << "\nSelect valid rotationType.\n";

    return iterGeneticAll();
  }*/
    if (algorithmOption == "1")
    {
        algorithnName = "Genetic Algoritm";
    }
    else if (algorithmOption == "2")
    {
        algorithnName = "Aritifical Bee Colony Algorithm";
    }
    else
    {
        algorithnName = "Firefly Algorithm";
    }

    if (rotationType == "1")
    {
        rotationType = "0";
    }
    else if (rotationType == "2")
    {
        rotationType = "2";
    }
    else
    {
        rotationType = "6";
    }
}

void handlePaths(bool isWithReplacement, string algorithm, string problemName, MutationType mutationTyped,
                 ROTATION_WAY rotation, string &fitnessDataPath, string &timePATH)
{
    string generalPath;

    generalPath = LOCAL_PATH + "/Results/" + problemName + "/" + getMutationName(mutationTyped) + "/" +
                  algorithm + "/" + getNameFromRotationWay(rotation) + "/";

    if (isWithReplacement)
    {
        if (algorithm == "GA")
        {
            fitnessDataPath = generalPath + algorithm + "Bin.csv";
            timePATH = generalPath + "tBin.csv";
        }
        else
        {
            fitnessDataPath = generalPath + algorithm + ".csv";
            timePATH = generalPath + "t.csv";
        }
    }
    else
    {

        if (algorithm == "GA")
        {
            fitnessDataPath = generalPath + algorithm + "2Bin.csv";
            timePATH = generalPath + "t2Bin.csv";
        }
        else
        {
            fitnessDataPath = generalPath + algorithm + "2.csv";
            timePATH = generalPath + "t2.csv";
        }
    }
}

void testDblfOneInstance()
{

    vector<DatasetBinBacking> DATASSET;
    const string dataPATH = LOCAL_PATH + "/Instance/P1A2.csv";
    try
    {
        DATASSET = readData(dataPATH);
    }

    catch (const exception &ex)
    {
        cerr << ex.what() << "\n";
    }
    DatasetBinBacking data = DATASSET[0];
    Chromosome chromosome{8, 9, 6, 12, 4, 15, 10, 11, 5, 16, 1, 14, 3, 13, 7, 2};
    Chromosome chromosomeRot{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    DoubleGenome genome = DoubleGenome::Build().setGenome(chromosome).setDGenome(chromosomeRot);
    Individuo ind = Individuo::Build().setGenome(genome);
    Individuo indQ = Individuo::Build().setGenome(genome);
    Bin bin = DATASSET[0].bin;
    Bin binQ = DATASSET[0].bin;

    DBLF(bin, ind.getGenome().getGenome(), ind.getGenome().getDGenome(), data.totalItems);
    ind.setFitness((double)bin.getLoadedVolume() / (double)bin.getDimensions().getVolumen());

    DBLFQueue(binQ, indQ.getGenome().getGenome(), indQ.getGenome().getDGenome(), data.totalItems);
    indQ.setFitness((double)binQ.getLoadedVolume() / (double)binQ.getDimensions().getVolumen());

    cout << ind.getFitness() << " | " << indQ.getFitness() << "\n";
}

void testRotationsGA(string name)
{
    print("GA with " + name + " evolving. ");

    vector<DatasetBinBacking> DATASSET;
    const string dataPATH = LOCAL_PATH + "/Instance/P1A2.csv";
    try
    {
        DATASSET = readData(dataPATH);
    }

    catch (const exception &ex)
    {
        cerr << ex.what() << "\n";
    }

    int maxIteration, population;
    double fitnessGAReplace, fitnessGAAdd, timeGAReplace, timeGAAdd;

    Poblacion bestPobRot;

    ROTATION_WAY rotation = ROTATION_WAY::SIX_WAY;
    MutationType mutationType = MutationType::C1;
    DatasetBinBacking data = DATASSET[0];

    maxIteration = 1000;
    population = 50;

    fitnessGAReplace = 0.0;
    fitnessGAAdd = 0.0;
    timeGAReplace = 0.0;
    timeGAAdd = 0.0;

    GeneticAlgorithm geneticAlgorithm = GeneticAlgorithm::Build()
                                            .setIsWithReplacement(true)
                                            .setMaxIteration(maxIteration)
                                            .setNumberOfIndividuals(population)
                                            .setRotationType(rotation)
                                            .setMutationType(mutationType)
                                            .setProblem(data)
                                            .setCrossProbability(0.75)
                                            .setSelectionProbability(0.85);
    for (int i = 0; i < 1000; i++)
    {
        cout << i << " ";

        auto start = chrono::steady_clock::now();
        bestPobRot = geneticAlgorithm.evolve();
        auto stop = chrono::steady_clock::now();
        auto duration = chrono::duration_cast<chrono::milliseconds>(stop - start);

        fitnessGAReplace += bestPobRot[0].getFitness() / 1000;
        timeGAReplace += duration.count() / 1000;
    }

    std::cout << "\nfi: " << fitnessGAReplace << "\n";
    std::cout << "t: " << (timeGAReplace / 1000) << " s\n";
}

void testDblf()
{

    vector<DatasetBinBacking> DATASSET;
    const string dataPATH = LOCAL_PATH + "/Instance/P5A2.csv";
    try
    {
        DATASSET = readData(dataPATH);
    }

    catch (const std::exception &ex)
    {
        cerr << ex.what() << "\n";
    }
    DatasetBinBacking data = DATASSET[0];
    Poblacion heuristicPoblation = buildHeuristicPoblation(1000, data.bin, data.totalItems);
    Poblacion secondPob = heuristicPoblation;
    auto start = std::chrono::steady_clock::now();

    for (Individuo &ind : heuristicPoblation)
    {
        Bin bin = data.bin;
        DBLF(bin, ind.getGenome().getGenome(), ind.getGenome().getDGenome(), data.totalItems);
        ind.setFitness((double)bin.getLoadedVolume() / (double)bin.getDimensions().getVolumen());
    }
    auto stop = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
    auto start2 = std::chrono::steady_clock::now();

    for (Individuo &ind : secondPob)
    {
        Bin bin = data.bin;
        DBLFQueue(bin, ind.getGenome().getGenome(), ind.getGenome().getDGenome(), data.totalItems);
        ind.setFitness((double)bin.getLoadedVolume() / (double)bin.getDimensions().getVolumen());
    }
    auto stop2 = std::chrono::steady_clock::now();
    auto duration2 = std::chrono::duration_cast<std::chrono::milliseconds>(stop2 - start2);

    for (int i = 0; i < secondPob.size(); i++)
    {
        if (heuristicPoblation[i].getFitness() != secondPob[i].getFitness())
        {
            std::cout << "not equal\n";
            std::cout << heuristicPoblation[i].getFitness() << "\n" << secondPob[i].getFitness() << "\n";
            std::cout << heuristicPoblation[i].getGenome() << "\n" << secondPob[i].getGenome() << "\n\n";
        }
    }
    std::cout << "DBLF: " << duration.count() << " | DBLFQueue: " << duration2.count() << "\n";
}

void evaluatePoblationTime()
{
    vector<DatasetBinBacking> DATASSET;
    const string dataPATH = LOCAL_PATH + "/Instance/P5A2.csv";
    try
    {
        DATASSET = readData(dataPATH);
    }

    catch (const std::exception &ex)
    {
        cerr << ex.what() << "\n";
    }
    int numberOfIndividuals = 100;
    DatasetBinBacking dataset = DATASSET[10];
    dataset.bin.setRotationWay(ROTATION_WAY::ZERO_WAY);
    // Get items ID list.
    LoadedBins totalItems = dataset.totalItems;
    // Builds an Heuristic poblation.
    Poblacion heuristicPoblation = buildHeuristicPoblation(
        /* numberPoblation= */ numberOfIndividuals,
        /* bin= */ dataset.bin,
        /* allItemsBin= */ totalItems);
    auto start = std::chrono::steady_clock::now();
    evaluatePoblation(heuristicPoblation, totalItems, dataset.bin);
    //  DBLF(dataset.bin, individual.getGenome().getGenome(), individual.getGenome().getDGenome(), totalItems);
    auto stop = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
    print("Duration: " << duration.count() << "ms");
    print("Item places: " << dataset.bin.getLoadedItems());
    print("FI: " << heuristicPoblation[0].getFitness());
    print("Genome: " << heuristicPoblation[0].getGenome());
}

void genetic()
{

    vector<DatasetBinBacking> DATASSET;
    const string dataPATH = LOCAL_PATH + "/Instance/P1A2.csv";
    try
    {
        DATASSET = readData(dataPATH);
    }

    catch (const std::exception &ex)
    {
        cerr << ex.what() << "\n";
    }
    auto start = std::chrono::steady_clock::now();
    std::cout << "Init\n";
    GeneticAlgorithm algorithm = GeneticAlgorithm::Build()
                                     .setIsWithReplacement(true)
                                     .setProblem(DATASSET[0])
                                     .setMaxIteration(1000)
                                     .setNumberOfIndividuals(50)
                                     .setCrossProbability(0.75)
                                     .setSelectionProbability(0.85)
                                     .setDMutationProbability(0.01)
                                     .setMutationType(MutationType::C2);
    Poblacion bestPob = algorithm.evolve();
    auto stop = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
    std::cout << "Initial best fit: " << bestPob[0].getFitness() << "\n";
    std::cout << "Initial worst fit: " << bestPob[bestPob.size() - 1].getFitness() << "\n";
    print("Duration: " << duration.count() << "ms");
}

std::string getAlgorithmNameFromNumber(std::string number)
{
    if (number == "1")
    {
        return "GA";
    }
    else if (number == "2")
    {
        return "ABC";
    }
    else
    {
        return "FFA";
    }
}

/** Gets evolutive algorithm based on the algorithm's name.*/
Poblacion getEvolutiveAlgorithm(std::string algorithm, ROTATION_WAY rotation, MutationType mutation,
                                DatasetBinBacking data, bool isWithReplacement)
{
    auto config = makeExperimentConfiguration(algorithm, rotation, mutation, isWithReplacement);
    config.candidates = configuredCandidates;

    return config.run(data);
}

/** Gets the mutation type by the user's input.*/
MutationType getMutationTypeFromUser(std::string mutationType)
{
    if (mutationType == "1")
    {
        return MutationType::INVERSE_MUTATION;
    }
    else if (mutationType == "2")
    {
        return MutationType::C1;
    }
    else
    {
        return MutationType::C2;
    }
}
