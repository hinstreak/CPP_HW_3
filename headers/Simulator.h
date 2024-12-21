#pragma once

#include <random>
#include <algorithm>
#include <cstring>
#include <fstream>

#include "Const.h"
#include "VectorField.h"
#include "InfoF.h"
#include "CusMatrix.h"
#include "ParsingSettings.h"

using std::tuple, std::pair, std::ofstream;

struct Simulator
{
    virtual void nextTick() = 0;
    virtual void init(const InfoF& f, const SimSetts& setts) = 0;
    virtual ~Simulator() = default;
};

template <typename pt, typename vt, typename vft, size_t Nv, size_t Mv>
struct SimulatorImpl final: Simulator
{
    VectorField<vt, Nv, Mv> velocity{};
    VectorField<vft, Nv, Mv> velocity_flow{};
    CusMatrix<pt, Nv, Mv> p{}, old_p{};
    CusMatrix<int64_t, Nv, Mv> last_use{}, dirs{};
    CusMatrix<uint8_t, Nv, Mv> field{};

    size_t N = Nv, M = Mv;
    pt rho[256]{};
    vt g{};
    int64_t UT = 0;
    std::mt19937 rnd;
    int64_t n_ticks{}, cur_tick{}; std::string out_name;

    SimulatorImpl();

    tuple<vft, bool, pair<int, int>> propagate_flow(int x, int y, vft lim);
    void propagate_stop(int x, int y, bool force = false);
    vt move_prob(int x, int y);
    void swap_between(int x, int y, int nx, int ny);
    bool propagate_move(int x, int y, bool is_first);
    vt random01();
    void directionsInit();
    void nextTick() override;
    void init(const InfoF& f, const SimSetts& setts) override;
    void serialize();
    ~SimulatorImpl() override = default;
};

template <typename pt, typename vt, typename vft, size_t Nv, size_t Mv>
void SimulatorImpl<pt, vt, vft, Nv, Mv>::init(const InfoF& f, const SimSetts& setts)
{
    g = f.g; N = f.height; M = f.width;
    for (int i = 0; i < 256; i++) {rho[i] = f.densities[i];}

    velocity.init(N, M);
    velocity_flow.init(N, M);
    p.init(N, M); old_p.init(N, M);
    last_use.init(N, M); dirs.init(N, M);
    field.init(N, M);

    for (size_t i = 0; i < N; i++) {
        for (size_t j = 0; j < M; j++) {
            field[i][j] = f.field[i][j];
        }
    }

    n_ticks = setts.n_ticks;
    out_name = setts.output_filename;

    directionsInit();
}

template <typename pt, typename vt, typename vft, size_t Nv, size_t Mv>
SimulatorImpl<pt, vt, vft, Nv, Mv>::SimulatorImpl(): rnd(1337) {}

template <typename pt, typename vt, typename vft, size_t Nv, size_t Mv>
void SimulatorImpl<pt, vt, vft, Nv, Mv>::directionsInit()
{
    for (size_t x = 0; x < N; ++x) {
        for (size_t y = 0; y < M; ++y) {
            if (field[x][y] == '#')
                continue;
            for (auto [dx, dy] : deltas) {
                dirs[x][y] += (field[x + dx][y + dy] != '#');
            }
        }
    }
}

template <typename pt, typename vt, typename vft, size_t Nv, size_t Mv>
void SimulatorImpl<pt, vt, vft, Nv, Mv>::swap_between(int x, int y, int nx, int ny)
{
    std::swap(field[x][y], field[nx][ny]);
    std::swap(p[x][y], p[nx][ny]);
    std::swap(velocity.v[x][y], velocity.v[nx][ny]);
}

template <typename pt, typename vt, typename vft, size_t Nv, size_t Mv>
vt SimulatorImpl<pt, vt, vft, Nv, Mv>::random01()
{
    if constexpr (std::is_floating_point_v<vt>) {
        return vt(rnd()) / vt(std::mt19937::max());
    }
    else {
        return vt::from_raw((rnd() & ((1ll << vt::k) - 1ll)));
    }
}

template <typename pt, typename vt, typename vft, size_t Nv, size_t Mv>
tuple<vft, bool, std::pair<int, int>> SimulatorImpl<pt, vt, vft, Nv, Mv>::propagate_flow(int x, int y, vft lim)
{
    last_use[x][y] = UT - 1;
    vft ret{};
    for (auto [dx, dy] : deltas)
    {
        int nx = x + dx, ny = y + dy;
        if (field[nx][ny] != '#' && last_use[nx][ny] < UT)
        {
            auto cap = velocity.get(x, y, dx, dy);
            auto flow = velocity_flow.get(x, y, dx, dy);
            if (fabs(double(flow - vft(cap))) <= 0.0001) continue;
            auto vp = std::min(lim, vft(cap) - flow);
            if (last_use[nx][ny] == UT - 1)
            {
                velocity_flow.add(x, y, dx, dy, vp);
                last_use[x][y] = UT;
                return {vp, 1, {nx, ny}};
            }
            auto [t, prop, end] = propagate_flow(nx, ny, vp);
            ret += t;
            if (prop)
            {
                velocity_flow.add(x, y, dx, dy, t);
                last_use[x][y] = UT;
                return {t, end != std::pair(x, y), end};
            }
        }
    }
    last_use[x][y] = UT;
    return {ret, false, {0, 0}};
}

template <typename pt, typename vt, typename vft, size_t Nv, size_t Mv>
void SimulatorImpl<pt, vt, vft, Nv, Mv>::propagate_stop(int x, int y, bool force)
{
    if (!force)
    {
        bool stop = true;
        for (auto [dx, dy] : deltas)
        {
            int nx = x + dx, ny = y + dy;
            if (field[nx][ny] != '#' && last_use[nx][ny] < UT - 1 && velocity.get(x, y, dx, dy) > int64_t(0)) {
                stop = false;
                break;
            }
        }
        if (!stop) return;
    }
    last_use[x][y] = UT;
    for (auto [dx, dy] : deltas)
    {
        int nx = x + dx, ny = y + dy;
        if (field[nx][ny] == '#' || last_use[nx][ny] == UT || velocity.get(x, y, dx, dy) > int64_t(0)) {
            continue;
        }
        propagate_stop(nx, ny);
    }
}

template <typename pt, typename vt, typename vft, size_t Nv, size_t Mv>
vt SimulatorImpl<pt, vt, vft, Nv, Mv>::move_prob(int x, int y)
{
    vt sum{};
    for (auto [dx, dy] : deltas)
    {
        int nx = x + dx, ny = y + dy;
        if (field[nx][ny] == '#' || last_use[nx][ny] == UT) {
            continue;
        }
        auto v = velocity.get(x, y, dx, dy);
        if (v < int64_t(0)) {
            continue;
        }
        sum += v;
    }
    return sum;
}


template <typename pt, typename vt, typename vft, size_t Nv, size_t Mv>
bool SimulatorImpl<pt, vt, vft, Nv, Mv>::propagate_move(int x, int y, bool is_first)
{
    last_use[x][y] = UT - is_first;
    bool ret = false;
    int nx = -1, ny = -1;
    do {
        std::array<vt, deltas.size()> tres;
        vt sum{};
        for (size_t i = 0; i < deltas.size(); ++i)
        {
            auto [dx, dy] = deltas[i];
            int nx = x + dx, ny = y + dy;
            if (field[nx][ny] == '#' || last_use[nx][ny] == UT) {
                tres[i] = sum;
                continue;
            }
            auto v = velocity.get(x, y, dx, dy);
            if (v < int64_t(0)) {
                tres[i] = sum;
                continue;
            }
            sum += v;
            tres[i] = sum;
        }

        if (sum == int64_t(0)) {
            break;
        }

        vt p = random01() * sum;
        size_t d = std::ranges::upper_bound(tres, p) - tres.begin();

        auto [dx, dy] = deltas[d];
        nx = x + dx;
        ny = y + dy;
        assert(velocity.get(x, y, dx, dy) > int64_t(0) && field[nx][ny] != '#' && last_use[nx][ny] < UT);

        ret = (last_use[nx][ny] == UT - 1 || propagate_move(nx, ny, false));
    } while (!ret);

    last_use[x][y] = UT;
    for (size_t i = 0; i < deltas.size(); ++i)
    {
        auto [dx, dy] = deltas[i];
        int nx = x + dx, ny = y + dy;
        if (field[nx][ny] != '#' && last_use[nx][ny] < UT - 1 && velocity.get(x, y, dx, dy) < int64_t(0)) {
            propagate_stop(nx, ny);
        }
    }
    if (ret)
    {
        if (!is_first) {
            swap_between(x, y, nx, ny);
        }
    }
    return ret;
}

template <typename pt, typename vt, typename vft, size_t Nv, size_t Mv>
void SimulatorImpl<pt, vt, vft, Nv, Mv>::nextTick()
{
    size_t x = 0;
    while (x < N) {
        size_t y = 0;
        while (y < M) {
            if (field[x][y] == '#') {
                ++y;
                continue;
            }
            if (field[x + 1][y] != '#') {
                velocity.add(x, y, 1, 0, g);
            }
            ++y;
        }
        ++x;
    }

    old_p = p;
    x = 0;
    while (x < N) {
        size_t y = 0;
        while (y < M)
        {
            if (field[x][y] == '#') continue;
            for (auto [dx, dy] : deltas)
            {
                int nx = x + dx, ny = y + dy;
                if (field[nx][ny] != '#' && old_p[nx][ny] < old_p[x][y])
                {
                    auto force = old_p[x][y] - old_p[nx][ny];
                    auto& contr = velocity.get(nx, ny, -dx, -dy);
                    if (pt(contr) * rho[(int) field[nx][ny]] >= force)
                    {
                        contr -= vt(force / rho[(int) field[nx][ny]]);
                        continue;
                    }
                    force -= pt(contr) * rho[(int) field[nx][ny]];
                    contr = int64_t(0);
                    velocity.add(x, y, dx, dy, vt(force / rho[(int) field[x][y]]));
                    p[x][y] -= force / pt(dirs[x][y]);
                }
            }
            ++y;
        }
        ++x;
    }

    velocity_flow.clear();

    bool prop;
    do {
        UT += 2;
        prop = false;
        size_t x = 0;
        size_t y = 0;
        while (x < N)
        {
            y = 0;
            while (y < M)
            {
                if (field[x][y] != '#' && last_use[x][y] != UT) {
                    auto [t, local_prop, _] = propagate_flow(x, y, int64_t(1));
                    if (t > int64_t(0)) {
                        prop = true;
                    }
                    ++y;
                }
            }
            ++x;
        }
    } while (prop);

    for (size_t x = 0; x < N; ++x) {
        for (size_t y = 0; y < M; ++y) {
            if (field[x][y] == '#')
                continue;
            for (auto [dx, dy] : deltas)
            {
                auto old_v = velocity.get(x, y, dx, dy);
                auto new_v = velocity_flow.get(x, y, dx, dy);
                if (old_v > int64_t(0))
                {
                    assert(vt(new_v) <= old_v);
                    velocity.get(x, y, dx, dy) = vt(new_v);
                    auto force = pt(old_v - vt(new_v)) * rho[(int) field[x][y]];
                    if (field[x][y] == '.')
                        force *= pt(0.8);
                    if (field[x + dx][y + dy] == '#') {
                        p[x][y] += force / pt(dirs[x][y]);
                    } else {
                        p[x + dx][y + dy] += force / pt(dirs[x + dx][y + dy]);
                    }
                }
            }
        }
    }

    UT += 2;
    prop = false;
    for (size_t x = 0; x < N; ++x) {
        for (size_t y = 0; y < M; ++y) {
            if (field[x][y] != '#' && last_use[x][y] != UT)
            {
                if (random01() < move_prob(x, y)) {
                    prop = true;
                    propagate_move(x, y, true);
                } else {
                    propagate_stop(x, y, true);
                }
            }
        }
    }

    if (prop)
    {
        for (size_t x = 0; x < N; ++x) {
            for (size_t y = 0; y < M; ++y) {
                std::cout << field[x][y];
            }
            std::cout << "\n";
        }
    }

    if (!out_name.empty() && (++cur_tick == n_ticks)) {
        serialize();
        cur_tick = 0;
    }
}

template <typename pt, typename vt, typename vft, size_t Nv, size_t Mv>
void SimulatorImpl<pt, vt, vft, Nv, Mv>::serialize()
{
    ofstream out(out_name);

    auto cnt = std::count_if(rho, rho+256, [](auto i){return i!=0l;});

    out << N << " " << M << " " << g << " " << cnt << "\n";
    int i = 0;
    while (i < 256)
    {
        if (rho[i] == 0l) continue;
        out << ((uint8_t)i) << " " << rho[i] << "\n";
        i++;
    }

    int x = 0;
    int y = 0;
    while (x < N)
    {
        y = 0;
        while (y < M)
        {
            out << field[x][y];
            y++;
        }
        out << "\n";
        x++;
    }
    out.close();
}