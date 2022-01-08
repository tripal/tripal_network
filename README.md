The Tripal Network Extension Module
===================================

[![Documentation Status](https://readthedocs.org/projects/tripal_network/badge/?version=latest)](https://tripal-network.readthedocs.io/en/latest/?badge=latest)
[![DOI](https://zenodo.org/badge/58517985.svg)](https://zenodo.org/badge/latestdoi/58517985)


The Tripal Network module supports inclusion of biological networks in a Tripal site. It provides:

- A variety of new Chado tables for storing networks and because Chado does not currently have tables for storing compounds and pathways the module adds those as well.  All of these new tables follow Chado design standards.
- A new Network content type.  Network pages describe the networks that have been added to the site.
- Tripal fields linking existing genomic features pages to networks.  The field provides a small interactive visualization of the first neighbor connections in the networks for a given feature.
- A full 3D application viewer for exploring the networks housed in the site.

For more information, please see the [online documentation](https://tripal-network.readthedocs.io/en/latest/)
