[![License](https://img.shields.io/badge/License-Apache_2.0-blue.svg)](https://opensource.org/licenses/Apache-2.0) ![CMake](https://github.com/balannarcis96/SkylakeLib/actions/workflows/cmake.yml/badge.svg?branch=main) ![Version](https://img.shields.io/badge/Version-v1.0.0.alpha-blue) [![Codacy Badge](https://app.codacy.com/project/badge/Grade/5bdf738cf8784fff846ceb7800f66a8e)](https://www.codacy.com/gh/balannarcis96/SkylakeLib/dashboard?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=balannarcis96/SkylakeLib&amp;utm_campaign=Badge_Grade)
# SkylakeLib 
[C++]Library containing basic building blocks for high performance servers.

- [SkylakeLibHeaderOnly][C++17] Header only part of the SkylakeLib
- [SkylakeLibStandalone][C++17] Standalone part of SkylakeLib
    - Intended to be used as part of client (eg Static library linked in UE5 project)
- [SkylakeLib][C++20] Library containing basic building blocks for high performance servers
   - Full SkylakeLibrary - Used as base library for building server applications
- [SkylakeLibDB][C++20] Mysql database abstractions and facilities 
- [SkylakeDatacenterLib][C++17] Datacenter facility
   - Allows for serializing of node-like data (eg. xml, json) into a flat binary file
   - Allows for node and attribute level data filtering and language based node filtering
   - With the filtering capabilities, separate binary files can be built from the same source data (eg. xml, json)
   - Extensible: Custom adaptors can be written to load different source data, Custom node, attributes filtering rules
   - Great for distributing client/server specific data
