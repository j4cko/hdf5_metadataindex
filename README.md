# HDF5 Index and Request Processing #

## Motivation ##

Large and very hierarchical HDF5 files are often expensive to traverse.
To process a request, every (or a large subset of) group and dataset must
be visited and the attribute compared to that listed in the request.
Most often (for large files), this comparison evaluates to false.
This is what makes file (meta-) operations expensive in analysis.

This project aims at visiting every group / dataset only once:
Attributes are extracted and saved to a database. After indexing,
a request can be processed in constant time and returns a full path
to the requested dataset without even opening the hdf5 file.

## DatasetSpecifications and Atomic Datatypes ##

Each Dataset is represented by a `DatasetSpec`, holding its attributes and file
information.
`Attributes` hold single, specific values, which can be of one of three types:

  * **numeric**
  * **boolean**
  * **string**
  * **array** (which is a associative container holding any of the other values)

Attributes are derived from the Hdf5 file: Datasets have their own (hdf5)
attributes and inherit all attributes of all their containing groups.
If a group contains a "table" dataset (i.e., it contains a dataset which has a
attribute "CLASS" holding the value "TABLE"), all datasets in the same group
(usually, there should be just one) are split into all positions described by
the table.

## Requests ##

A request consists of several sections (each can be omitted)

  * `AttributeRequest`s demand that attributes fulfill certain conditions,
  * `FileRequest`s can be used to filter for file name and date,
  * `Hdf5DatasetRequest` filters for dataset names and the inner structure of
    the file.
  * `LuaRequest`s are currently not implemented but will allow to have more
    complex logic in requests.

A request is a list of `Conditions`. More than one dataset can match a request. 
Datasets can have attributes set that are not part of the request 
(for which no corresponding AttributeRequest exists).

In other words: A dataset matching a request has all the attributes
of the request set to a value for which the AttributeRequest returns true (and
similarly for the file information). It may have additional attributes, 
but all attributes in the request are set and have the right value.

Requests can be currently be parsed from json. The keywords `attributes`, `file`
and `luacode` (as soon as implemented) refer to the individual request sections.

### Evaluation hierarchy ###

The order in which requests are evaluated is not fixed: This is no problem since
requests are connected by an (implicit) "and" (all requests need to be
fulfilled). This leaves room for performance considerations: In practice, some
kind of "preselection", only evaluating a subset of all requests, can be done,
before a "postselection" will only be evaluated on the datasets that match the
preselection rules. For example, lua code would only be executed on the datasets
that fulfill all other requirements. Thus, from the user perspective, it might
be beneficial to specify as many requests as possible (because only the
potentially fastest could be evaluated).

### Examples ###

  * `{"attributes": {"attrname": 3}}` checks if the attribute named `attrname` 
    exists and is set to 3.
  * `{"attributes": {"attrname": {"min": 2}}` checks if the attribute `attrname`
    exists and is set to a value larger or equal to 2.
  * similarly: `{"attributes": {"attrname": {"max": 2}}}`
  * `{"attributes": {"attrname": {"min": 1, "max": 3}}}` checks if the attribute 
    `attrname` is set to a value between 1 and 3 (boundaries included).
  * `{"attributes": {"attrname": {"or": [1,2,3]}}}` check if the attribute 
    `attrname` is set to one of the values 1, 2 or 3.
  * `{"attributes": {"attrname": {"present": true}}}` check if the attribute 
    `attrname` is present and set to any value. Note that the reverse, 
    `{"attrname": {"present": false}}` which would check for the absence of an
    attribute, is currently not implemented.
  * `{"attributes": {"attrname": {"not": 1}}}` returns all datasets that have
    the attribute `attrname` set but not to the value 1.
  * `{"attributes": {"attrname": {"not": 1}, "attrname": {"not": 2}}}` returns 
    all datasets that have the attribute `attrname` set but not to the values 1
    nor 2.
  * `{"attributes": {"attrname": {"matches": ".*numerated[0-9]*"}}}`: also
    regexes work (also on numeric types!)
  * `{"file": {"newer": 1480004355}}` requests modification time of the datafile
    to be newer than Do 24. Nov 17:19:13 CET 2016
  * similarly, `older` for "older than" and `mtime` for "exactly from" work for
    modification times.
  * `{"file": {"matches": ".*H101r001n1.h5$"}}` would return all datasets that
    can be found in files that end with this string (might be useful to restrict
    to certain configs). Of course any regex works.
  * Several sections can be combined: `{"attribute": {"attrname": 3}, "file":
    {"matches": ".*outputfile"}}` returns all datasets in files that match the
    regex and have an attribute `attrname` set to 3.

#### Future ideas: Additional attribute requests ####

In addition to the above requests, one could also have:

  * `"attr": {"present": false }` matches any node which does **not** have the 
    attribute `attr`.
  * `"attr": {"smallest": true}` and `"attr": {"largest": true}` matches all nodes that have
    the attribute `attr` set to the smallest or largest value (compared to all
    other nodes which hold this attribute). The values `true` are required.

#### Future ideas: Lua ####

`luacode` as an additional request section could be added which is
executed (only) on every node that fulfills the requirements of the other 
specifiers. It gets the DatasetSpec of the node as tables.

For illustration, assume we have three nodes:
```
{
  "attributes": {"x": 2, "y": 2, "z": 3, "t": 2},
  "file": "targetnode1"
}
{
  "attributes": {"x": 3, "y": 3 "z": 3, "t": 2},
  "file": "targetnode2"
}
{
  "attributes": {"x": 3, "y": 3 "z": 3, "t": 5},
  "file": "targetnode3"
}
```
then the request
```
{
  "attributes": {"t": 2},
  "file": {"matches": "targetnode[0-9]*"},
  "luacode": "function() return attributes.x^2+attributes.y^2+attributes.z^2==27 end"
}
```
would match only the second: the first two match the preselector (the attributes
tag), the third one *would* match the lua code but it is not executed on it
because the preselector does not match (t is 5 and not 2).

## Build instructions ##

Building is currently done by a simple Makefile (no build system yet).
Dependencies include: `hdf5` and `sqlite3` for the indexer and
`sqlite3` and `jsoncpp` for the queryDb tool.

## Examples ##

Index a hdf5 file:
```./indexHdf5 hdf5file.h5 dbfile.sqlite```

The database can then be queried with the `queryDb` tool:
```./queryDb dbfile.sqlite '{"attributes": {"px" : 1, "py" : 0, "pz" : 0}}'```
(this will return all datasets that have the attributes `px`, `py` and `pz` set
to these values, see above for more examples).

## Other ToDo ##
  * provide a better **CLI interface**:
    - output formats: print all results with all attributes, print only
      filenames, only datasetnames, both together, ...
