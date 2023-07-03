
  @mainpage Introdution & Guides
  # Welcome to the document of HRCore :-)
  HRCore is a rough implement of high resolution calculation library, as a part of my C++ course big homework at the first term of SCUT.
 
  It has implemented integer and float datatype, along with `+ - * / %` operation, with two types of implement of storage structure.
 
  ## Features
   - Almost unlimited data length
   - High resolution, with integers and floats
   - Limited calculation operator provided
   - Expression calculation supported
   - Support for std::cin & std::cout
 
  ## About the structure
  Just shown below:
  @dot General Structure
  digraph example {
       node [shape=record, fontname=Helvetica, fontsize=10];
       rankdir = BT;
       Integer [ label="Integer" URL="\ref HRCore::Value::Integer"];
       FixedPoint [ label="FixedPoint" URL="\ref HRCore::Value::FixedPoint"];
       SIF [label = "Storage::Interface" URL="\ref HRCore::Storage::Interface" ];
       LinkedList [ label="LinkedList" URL="\ref HRCore::Storage::LinkedList"];
       VirtualMMU [ label="VirtualMMU" URL="\ref HRCore::Storage::VirtualMMU"];
 
       LinkedList -> SIF[style = "dashed"];
       VirtualMMU -> SIF[style = "dashed"];
       SIF -> Integer;
       Integer -> FixedPoint;
   }
  @enddot
 
  # How to build
  This library is header only and actually not need to build specially. Just include the file "HRCore.h" and compile, and that's it!
 
  If you'd like to make a standalone header file, please your setup meet the following requirements: \n
  Toolchain requirements:
   - GNU C Compiler that supports for C++11 standard (With C++0x may be OK)
   - CMake
   - GNU Make
  
  Then, run cmake to configure the build and execute "make merge" in the build directory you have just specified.
 
  Actually, the library is cross-platform and MSVC is also supported.
 
  # Examples
 
  ## Example 01: The usage of Integer / Float 
  @include{lineno} example_01.cpp
 
 
  ## Example 02 The usage of Float::precision()
  @include{lineno} example_02.cpp
 
 
