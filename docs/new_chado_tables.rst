New Chado Tables
================

Tables for Storing Compounds
----------------------------

The compound Table
``````````````````
The *compound* table is analogous to the Chado feature table although with fewer fields and is meant to house compounds. The table is minimal and may change as the module evolves.  The *name* field must be a unique value for each file and thus can be selected on for finding files.

+-------------+------------+------+---------------+---------------------------+
| Column      | Type       | Null | Default Value | Constraint                |
+=============+============+======+===============+===========================+
| compound_id | integer    | No   | (auto)        | Primary Key               |
+-------------+------------+------+---------------+---------------------------+
| name        | text       | No   |               |                           |
+-------------+------------+------+---------------+---------------------------+
| uniquename  | integer    | No   |               | Unique                    |
+-------------+------------+------+---------------+---------------------------+

The compoundprop Table
``````````````````````

The compound Table
``````````````````
The compound_dbxref Table
`````````````````````````
The compound_synonym Table
``````````````````````````

Tables for Storing Reactions
----------------------------

The reaction Table
``````````````````

The reactionprop Table
```````````````````````

The reaction_substrate Table
`````````````````````````````

The reaction_product Table
```````````````````````````


Tables for Storing Pathways
---------------------------

The pathway Table
``````````````````

The pathwayprop Table
``````````````````````

The pathway_dbxref Table
````````````````````````

The pathway_reaction Table
``````````````````````````

The pathway_feature Table
`````````````````````````

Tables for Storing Networks
---------------------------

The network Table
``````````````````

The networkprop Table
``````````````````````

The network_attr Table
``````````````````````

The network_attrprop Table
``````````````````````````

The network_cvterm Table
`````````````````````````

The network_file Table
```````````````````````

The network_node Table
``````````````````````
The network_nodeprop Table
``````````````````````````

The network_feature Table
``````````````````````````

The network_compound Table
``````````````````````````

The network_edge Table
```````````````````````

The network_edgeprop Table
``````````````````````````

The network_analysis Table
``````````````````````````

The network_analysisprop Table
``````````````````````````````

The network_pub Table
``````````````````````
The network_layout Table
````````````````````````
