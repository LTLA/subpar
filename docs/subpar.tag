<?xml version='1.0' encoding='UTF-8' standalone='yes' ?>
<tagfile doxygen_version="1.12.0">
  <compound kind="file">
    <name>range.hpp</name>
    <path>subpar/</path>
    <filename>range_8hpp.html</filename>
    <namespace>subpar</namespace>
  </compound>
  <compound kind="file">
    <name>simple.hpp</name>
    <path>subpar/</path>
    <filename>simple_8hpp.html</filename>
    <includes id="range_8hpp" name="range.hpp" local="yes" import="no" module="no" objc="no">range.hpp</includes>
    <namespace>subpar</namespace>
  </compound>
  <compound kind="file">
    <name>subpar.hpp</name>
    <path>subpar/</path>
    <filename>subpar_8hpp.html</filename>
    <includes id="range_8hpp" name="range.hpp" local="yes" import="no" module="no" objc="no">range.hpp</includes>
    <includes id="simple_8hpp" name="simple.hpp" local="yes" import="no" module="no" objc="no">simple.hpp</includes>
    <namespace>subpar</namespace>
  </compound>
  <compound kind="namespace">
    <name>subpar</name>
    <filename>namespacesubpar.html</filename>
    <member kind="function">
      <type>void</type>
      <name>parallelize_simple</name>
      <anchorfile>namespacesubpar.html</anchorfile>
      <anchor>ab039ee0d9f8d12394d4c59bb994c98ca</anchor>
      <arglist>(const Task_ num_tasks, const Run_ run_task)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>sanitize_num_workers</name>
      <anchorfile>namespacesubpar.html</anchorfile>
      <anchor>a7f48f62226f93e5bf735ce35220bcd09</anchor>
      <arglist>(const int num_workers, const Task_ num_tasks)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>parallelize_range</name>
      <anchorfile>namespacesubpar.html</anchorfile>
      <anchor>a6299ecb1a7d6184a37af21dfe09cc92e</anchor>
      <arglist>(int num_workers, const Task_ num_tasks, const Run_ run_task_range)</arglist>
    </member>
  </compound>
  <compound kind="page">
    <name>index</name>
    <title>Substitutable parallelization for C++ libraries</title>
    <filename>index.html</filename>
    <docanchor file="index.html" title="Substitutable parallelization for C++ libraries">md__2github_2workspace_2README</docanchor>
  </compound>
</tagfile>
