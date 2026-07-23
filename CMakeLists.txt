cmake_minimum_required(VERSION 3.10)
project(quotient VERSION 1.0 LANGUAGES CXX)

# -------------------------------
# Standard i opcje kompilatora
# -------------------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if (ENABLE_ASAN)                                                                                                                                                                             
    message(STATUS "Building with AddressSanitizer")                                                                                                                                         
    add_compile_options(                                                                                                                                                                     
        -g                                                                                                                                                                                   
                -O1                                                                                                                                                                          
               -fno-omit-frame-pointer                                                                                                                                                       
        -fsanitize=address                                                                                                                                                                   
         )                                                                                                                                                                                   
         add_link_options(                                                                                                                                                                   
        -fsanitize=address                                                                                                                                                                   
        )                                                                                                                                                                                    
        endif()                                                                                                                                                                              
        if (ENABLE_UBSAN)                                                                                                                                                                    
           message(STATUS "Building with UBSanitizer")                                                                                                                                       
        add_compile_options(                                                                                                                                                                 
        -g                                                                                                                                                                                   
        -O1                                                                                                                                                                                  
        -fno-omit-frame-pointer                                                                                                                                                              
        -fsanitize=undefined                                                                                                                                                                 
         )                                                                                                                                                                                   
                                                                                                                                                                                              
        add_link_options(                                                                                                                                                                    
        -fsanitize=undefined                                                                                                                                                                 
        )                                                                                                                                                                                    
        endif()     
# Opcje CPU
add_compile_options(-march=native -mavx2 -mfma)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# -------------------------------
# Znajdź wymagane biblioteki
# -------------------------------
find_library(DIVSUFSORT_LIB divsufsort)
if(NOT DIVSUFSORT_LIB)
    message(FATAL_ERROR "libdivsufsort not found")
endif()

find_package(OpenMP REQUIRED)

add_custom_target(psascan_build ALL
    COMMAND make
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/externals/psascan
)

# -------------------------------
# Dodaj katalogi include
# -------------------------------
include_directories(
    include
    ${CMAKE_SOURCE_DIR}/externals/unordered_dense/include
)


# -------------------------------
# Dodaj libsais jako subdirectory
# -------------------------------
set(LIBSAIS_USE_OPENMP ON CACHE BOOL "Use OpenMP for parallelization" FORCE)
add_subdirectory(externals/libsais)

# -------------------------------
# Twoje biblioteki
# -------------------------------
add_library(FastaUtils
    src/fasta_utils.cpp
)
add_library(SAMPLE
	src/build_samples.cpp)

add_library(SequenceUtils
    src/sequence_utils.cpp
)

add_library(QUOTIENT_E
    src/quotient_external.cpp
)
target_compile_definitions(QUOTIENT_E PRIVATE
    CONSTRUCT_SA_PATH="${CMAKE_SOURCE_DIR}/externals/psascan/construct_sa"
)

target_link_libraries(QUOTIENT_E
    PRIVATE SAMPLE
)

add_library(Indexing
	src/Indexing.cpp
)
target_include_directories(Indexing
    PRIVATE
        ${CMAKE_SOURCE_DIR}/externals/libsais
)

target_link_libraries(Indexing
    PRIVATE libsais
)



# -------------------------------
# Tworzenie głównego executabla
# -------------------------------
add_executable(quotient main.cpp src/build_external.cpp src/build_rapid.cpp)
target_compile_definitions(quotient PUBLIC
    CONSTRUCT_SA_PATH="${CMAKE_SOURCE_DIR}/externals/psascan/construct_sa"
)

target_link_libraries(quotient
    PRIVATE FastaUtils
    PRIVATE SequenceUtils
    PRIVATE ${DIVSUFSORT_LIB}
    PRIVATE libsais           # libsais z add_subdirectory
    PRIVATE OpenMP::OpenMP_CXX
    PRIVATE SAMPLE
    PRIVATE QUOTIENT_E
    PRIVATE Indexing
)

# -------------------------------
# Ustawienie include dla libsais
# -------------------------------
target_include_directories(quotient
    PRIVATE ${CMAKE_SOURCE_DIR}/externals/libsais/include
)

# -------------------------------
# Ustawienia kompilacji dla FastaUtils i SequenceUtils
# -------------------------------
target_link_libraries(FastaUtils PRIVATE libsais OpenMP::OpenMP_CXX)
target_link_libraries(SequenceUtils PRIVATE libsais OpenMP::OpenMP_CXX)

# -------------------------------
# Dodaj definicję preprocesora LIBSAIS_OPENMP
# -------------------------------
# libsais już ustawia ją jako PUBLIC, ale na wszelki wypadek:
target_compile_definitions(quotient PRIVATE LIBSAIS_OPENMP)

# -------------------------------
# Opcjonalnie: kompilacja z sdsl, jeśli używasz
# -------------------------------
#find_package(sdsl REQUIRED)
target_link_libraries(quotient PRIVATE sdsl)
