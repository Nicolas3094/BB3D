#ifndef MAXIMAL_SPACES_PROFILE_HPP
#define MAXIMAL_SPACES_PROFILE_HPP

#include <chrono>
#include <cstddef>
#include <cstdint>

/**
 * @brief Mediciones de una evaluacion, habilitadas solo en compilaciones de perfil.
 * @note Las fases son excluyentes. La division incluye ensamblar las regiones;
 * la poda incluye contencion y duplicados porque comparten el mismo bucle.
 */
struct MaximalSpacesProfile
{
    double placementMicroseconds = 0;
    double splitMicroseconds = 0;
    double pruningMicroseconds = 0;
    double sortingMicroseconds = 0;
    std::uint64_t candidateSpaces = 0;
    std::uint64_t spacesVisitedForSplit = 0;
    std::uint64_t affectedSpaces = 0;
    std::uint64_t pruningPairVisits = 0;
    std::uint64_t containmentChecks = 0;
    std::uint64_t duplicatesRemoved = 0;
    std::uint64_t containedSpacesRemoved = 0;
    std::size_t peakSpacesBeforePruning = 0;
    std::size_t peakRetainedSpaces = 1;
};

/** @brief Accede al perfil del hilo actual; el ejecutor lo reinicia por evaluacion. */
inline MaximalSpacesProfile &maximalSpacesProfile()
{
    static thread_local MaximalSpacesProfile profile;
    return profile;
}

/** @brief Acumula tiempo transcurrido en una fase, con cierre explicito o por alcance. */
class MaximalSpacesPhaseTimer
{
    using Clock = std::chrono::steady_clock;
    double &microseconds;
    Clock::time_point start = Clock::now();
    bool running = true;

public:
    explicit MaximalSpacesPhaseTimer(double &destination) : microseconds(destination) {}
    ~MaximalSpacesPhaseTimer() { finish(); }

    /** @brief Cierra la medicion una sola vez, antes de iniciar la siguiente fase. */
    void finish()
    {
        if (!running)
            return;
        microseconds += std::chrono::duration<double, std::micro>(Clock::now() - start).count();
        running = false;
    }
};

#endif
