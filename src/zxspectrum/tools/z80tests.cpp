/* This file is part of Epoch, Copyright (C) 2024 Andrea Ghidini.
 *
 * Epoch is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Epoch is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Epoch.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <set>

#include <nlohmann/json.hpp>

#include "utils.hpp"

std::set<std::filesystem::path> findJsons(const std::filesystem::path& path)
{
    std::set<std::filesystem::path> results{};
    if (!std::filesystem::is_directory(path)) return results;
    for (const auto& entry : std::filesystem::directory_iterator(path))
    {
        if (entry.is_regular_file() && entry.path().extension() == ".json")
        {
            results.insert(entry);
        }
    }
    return results;
}

struct TestInfo
{
    struct State
    {
        uint16_t pc;
        uint16_t sp;
        uint8_t a;
        uint8_t b;
        uint8_t c;
        uint8_t d;
        uint8_t e;
        uint8_t f;
        uint8_t h;
        uint8_t l;
        uint8_t i;
        uint8_t r;
        bool ei;
        uint16_t wz;
        uint16_t ix;
        uint16_t iy;
        uint16_t af_;
        uint16_t bc_;
        uint16_t de_;
        uint16_t hl_;
        uint8_t im;
        bool p;
        bool q;
        bool iff1;
        bool iff2;
        std::vector<std::pair<uint16_t, uint8_t>> ram;
    };

    std::string name;
    State initial;
    State final;
    int cycles;
    std::vector<RamZ80Interface::IoOperation> ports;
};

void from_json(const nlohmann::json& j, TestInfo::State& r)
{
    j.at("pc").get_to(r.pc);
    j.at("sp").get_to(r.sp);
    j.at("a").get_to(r.a);
    j.at("b").get_to(r.b);
    j.at("c").get_to(r.c);
    j.at("d").get_to(r.d);
    j.at("e").get_to(r.e);
    j.at("f").get_to(r.f);
    j.at("h").get_to(r.h);
    j.at("l").get_to(r.l);
    j.at("i").get_to(r.i);
    j.at("r").get_to(r.r);
    r.ei = j.at("ei").get<int>();
    j.at("wz").get_to(r.wz);
    j.at("ix").get_to(r.ix);
    j.at("iy").get_to(r.iy);
    j.at("af_").get_to(r.af_);
    j.at("bc_").get_to(r.bc_);
    j.at("de_").get_to(r.de_);
    j.at("hl_").get_to(r.hl_);
    j.at("im").get_to(r.im);
    r.p = j.at("p").get<int>();
    r.q = j.at("q").get<int>();
    r.iff1 = j.at("iff1").get<int>();
    r.iff2 = j.at("iff2").get<int>();
    for (const auto& it : j.at("ram"))
    {
        const auto addr = static_cast<uint16_t>(it[0].get<int>());
        const auto value = static_cast<uint8_t>(it[1].get<int>());
        r.ram.emplace_back(addr, value);
    }
}

void from_json(const nlohmann::json& j, TestInfo& r)
{
    j.at("name").get_to(r.name);
    j.at("initial").get_to(r.initial);
    j.at("final").get_to(r.final);
    r.cycles = static_cast<int>(j["cycles"].size());
    if (j.contains("ports"))
    {
        for (const auto& it : j.at("ports"))
        {
            const auto addr = static_cast<uint16_t>(it[0].get<int>());
            const auto value = static_cast<uint8_t>(it[1].get<int>());
            const auto direction = it[2].get<std::string>();
            r.ports.emplace_back(addr, value, direction == "w");
        }
    }
}

#define CHECK_VALUE(key, width, actual, expected) \
    if ((actual) != (expected)) \
    { \
        std::cout << "Test " << testInfo.name << " KO\t" << key << "\texpected: 0x" << std::hex<<std::setw((width)*2)<<std::setfill('0') << (int)(expected) << "\tactual: 0x" << std::hex<<std::setw((width)*2)<<std::setfill('0') << (int)(actual) << "\n"; \
        success = false; \
    } else

bool executeTest(RamZ80Interface& interface, epoch::zxspectrum::Z80Cpu& cpu, const TestInfo& testInfo)
{
    cpu.reset();

    cpu.registers().pc = testInfo.initial.pc;
    cpu.registers().sp = testInfo.initial.sp;
    cpu.registers().af.high = testInfo.initial.a;
    cpu.registers().af.low = testInfo.initial.f;
    cpu.registers().bc.high = testInfo.initial.b;
    cpu.registers().bc.low = testInfo.initial.c;
    cpu.registers().de.high = testInfo.initial.d;
    cpu.registers().de.low = testInfo.initial.e;
    cpu.registers().hl.high = testInfo.initial.h;
    cpu.registers().hl.low = testInfo.initial.l;
    cpu.registers().ir.high = testInfo.initial.i;
    cpu.registers().ir.low = testInfo.initial.r;
    // EI (?)
    cpu.registers().wz = testInfo.initial.wz;
    cpu.registers().ix = testInfo.initial.ix;
    cpu.registers().iy = testInfo.initial.iy;
    cpu.registers().af2 = testInfo.initial.af_;
    cpu.registers().bc2 = testInfo.initial.bc_;
    cpu.registers().de2 = testInfo.initial.de_;
    cpu.registers().hl2 = testInfo.initial.hl_;
    cpu.registers().interruptMode = testInfo.initial.im;
    cpu.registers().iff1 = testInfo.initial.iff1;
    cpu.registers().iff2 = testInfo.initial.iff2;
    for (const auto& [address, value] : testInfo.initial.ram)
    {
        interface.ram()[address] = value;
    }
    interface.setIoOperations(testInfo.ports);
    auto success = true;

    try
    {
        for (auto i = 0; i < testInfo.cycles; i++)
        {
            cpu.clock();
        }
    }
    catch (std::runtime_error& err)
    {
        std::cout << "Test " << testInfo.name << " KO\t" << err.what() << "\n";
        success = false;
    }

    CHECK_VALUE("PC", 2, cpu.registers().pc, testInfo.final.pc);
    CHECK_VALUE("SP", 2, cpu.registers().sp, testInfo.final.sp);
    CHECK_VALUE("A", 1, cpu.registers().af.high, testInfo.final.a);
    CHECK_VALUE("F", 1, cpu.registers().af.low, testInfo.final.f);
    CHECK_VALUE("B", 1, cpu.registers().bc.high, testInfo.final.b);
    CHECK_VALUE("C", 1, cpu.registers().bc.low, testInfo.final.c);
    CHECK_VALUE("D", 1, cpu.registers().de.high, testInfo.final.d);
    CHECK_VALUE("E", 1, cpu.registers().de.low, testInfo.final.e);
    CHECK_VALUE("H", 1, cpu.registers().hl.high, testInfo.final.h);
    CHECK_VALUE("L", 1, cpu.registers().hl.low, testInfo.final.l);
    CHECK_VALUE("I", 1, cpu.registers().ir.high, testInfo.final.i);
    CHECK_VALUE("R", 1, cpu.registers().ir.low, testInfo.final.r);
    CHECK_VALUE("WZ", 2, cpu.registers().wz, testInfo.final.wz);
    CHECK_VALUE("IX", 2, cpu.registers().ix, testInfo.final.ix);
    CHECK_VALUE("IY", 2, cpu.registers().iy, testInfo.final.iy);
    CHECK_VALUE("AF2", 2, cpu.registers().af2, testInfo.final.af_);
    CHECK_VALUE("BC2", 2, cpu.registers().bc2, testInfo.final.bc_);
    CHECK_VALUE("DE2", 2, cpu.registers().de2, testInfo.final.de_);
    CHECK_VALUE("HL2", 2, cpu.registers().hl2, testInfo.final.hl_);
    CHECK_VALUE("IM", 1, cpu.registers().interruptMode, testInfo.final.im);
    CHECK_VALUE("IFF1", 1, cpu.registers().iff1, testInfo.final.iff1);
    CHECK_VALUE("IFF2", 1, cpu.registers().iff2, testInfo.final.iff2);

    for (const auto& [address, value] : testInfo.final.ram)
    {
        CHECK_VALUE("RAM[0x"<<std::hex<<std::setw(4)<<std::setfill('0')<<address<<"]", 1, interface.ram()[address], value);
    }

    return success;
}

int executeTestSuite(RamZ80Interface& interface, epoch::zxspectrum::Z80Cpu& cpu, const std::filesystem::path& testSuitePath)
{
    std::ifstream fs(testSuitePath);
    const auto j = nlohmann::json::parse(fs);
    auto failed = 0;
    for (const auto& it : j)
    {
        const auto testInfo = it.get<TestInfo>();
        if (!executeTest(interface, cpu, testInfo))
        {
            failed++;
        }
    }
    return failed;
}

int main()
{
    const std::filesystem::path path = std::filesystem::weakly_canonical("z80/v1");
    const auto files = findJsons(path);
    if (files.empty())
    {
        std::cerr << "No input files found in " << path << "\n";
        std::cout << "Please download Z80 json tests from https://github.com/raddad772/jsmoo/tree/main/misc/tests/GeneratedTests/z80\n";
        return EXIT_FAILURE;
    }
    std::cout << "Found " << files.size() << " files in " << path << std::endl;

    RamZ80Interface interface;
    epoch::zxspectrum::Z80Cpu cpu{ interface };

    const auto startTime = std::chrono::high_resolution_clock::now();

    auto remaining = files.size();
    auto failed = 0;
    for (const auto& testSuitePath : files)
    {
        remaining--;
        // if (testSuitePath.filename().generic_string().starts_with("de"))
        failed += executeTestSuite(interface, cpu, testSuitePath);
    }

    const auto stopTime = std::chrono::high_resolution_clock::now();
    const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stopTime - startTime);
    const auto durationSec = static_cast<double>(duration.count()) / 1e6;
    std::cout << "\n";
    std::cout << "=======================================\n";
    std::cout << "Duration:     " << durationSec << " s\n";
    std::cout << "Failed tests: " << std::dec << failed << "\n";

    return failed > 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}
