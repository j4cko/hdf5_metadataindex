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

## Requests ##

### Atomic Datatypes ###

Attributes hold values that compare to values set in the target files. Requests
distinguish only between

  * **numeric**
  * **boolean**
  * **string**

types and lists thereof.


### At the moment ###

A request is a list of attributes with certain values. More than one
dataset can match a request. Datasets can have attributes that are not 
part of the request.

In other words: A dataset matching a request has all the attributes
of the request set to the same values as in the request. It may have
additional attributes, but all attributes in the request are set 
and have the right value.

*Wildcarding* is naturally done by not specifying the attribute in the
request.

### Future / Proposal ###

In JSON, a request could look like this:
```
{
  "attributes": {"attr1": 1, "attr2": "example"},
  "name": ".*/measurementname"
}
```
This request returns all nodes (entities, datasets) which have **all**
attributes of the request set to the values specified in the request.

The additional (optional) `name` tag is a regex that must match the name of the
node.

#### Attribute keywords ####

Attributes set to a fixed value must match exactly.
The matching of attributes could be weakened by additional keywords:

  * `"attr": 1` matches only nodes that have a attributes named `attr` which 
    is set to `1`.
  * `"attr": {"min": 1, "max": 2}` would match any node with an attribute named 
    `attr` with a numeric value between `1` and `2`.
  * `"attr": {"min": 1}` would match any node larger or equal 1, similarly does
    `"attr": {"max": 3}` match all nodes with `attr` smaller or equal 3.
  * `"attr": {"or": 1, "or": 2, "or": 3}` matches either `1`, `2` or `3`.
  * `"attr": {"present": true}` matches any node which has the attribute `attr`
    (irrespective of its value). If set to `false`, it doesn't match any node
    which has this attribute set.
  * `"attr": {"smallest": true}` and `"attr": {"largest": true}` matches all nodes that have
    the attribute `attr` set to the smallest or largest value (compared to all
    other nodes which hold this attribute). The values `true` are required.

Note the enclosing braces: This is needed to distinguish the attribute from a
string.

#### Lua ####

If needed, this could be complemented by another element, `luacode`, which is
executed (only) on every node that fulfills the requirements of the other 
specifiers. It gets the specifications of the node as tables.

#### Example ####
Assume we have three nodes:
```
{
  "attributes": {"x": 2, "y": 2, "z": 3, "t": 2},
  "name": "targetnode1"
}
{
  "attributes": {"x": 3, "y": 3 "z": 3, "t": 2},
  "name": "targetnode2"
}
{
  "attributes": {"x": 3, "y": 3 "z": 3, "t": 5},
  "name": "targetnode3"
}
```
then the request
```
{
  "attributes": {"t": 2},
  "name": "targetnode[0-9]*",
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
```./queryDb dbfile.sqlite '{"px" : 1, "py" : 0, "pz" : 0}'```
(this will return all datasets that have the attributes `px`, `py` and `pz` set
to these values)

## maybe some time... ##

  * **more logic in requests**: 
    specifiers like "not", "one-of", etc. to attributes might be 
    interesting. Also ranges are a natural extension.
  * **notice if the file has changed**
    save the file mtime and checksum in the database and 
    update / refuse if the index is not in sync with the file.
  * **inter-file support**
    make databases cover several files: The database can then
    also be used to create file lists. 
    Example: "get all messpec data (kappa=...) of H101" returns a list
    of files and datasets that return relevant data.
  * **regex matching on dataset names**
  * **lua-post-selection**:
    after preselection by matching of the attributes, lua code is executed on
    every hit. If it returns `true`, the hit is returned, otherwise it is
    discarded.
  * **HDF5BaseTable**
    (as an object in lua?)
