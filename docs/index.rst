Tripal Network Extension Module
===============================

.. warning::

  The Tripal Network module is currently in alpha release mode.  If you find bugs please report them on the `GitHub issue queue <https://github.com/tripal/tripal_network/issues>`_

The Tripal Network module supports inclusion of biological networks in a Tripal site. It provides:

- A variety of :doc:`new_chado_tables` for storing networks and because Chado does not currently have tables for storing compounds and pathways the module adds those as well.  All of these new tables follow Chado design standards.
- A new Network content type.  Network pages describe the networks that have been added to the site.
- Tripal fields linking existing genomic features pages to networks.  The field provides a small interactive visualization of the first neighbor connections in the networks for a given feature.
- A full 3D application viewer for exploring the networks housed in the site.

.. warning::

  The Tripal Network module is **compatible only with Tripal v3.6 or higher**.

.. toctree::
   :maxdepth: 4
   :caption: Tripal Network Extension Module

   ./install
   ./prepare_data
   ./load_data
   ./visualization
   
