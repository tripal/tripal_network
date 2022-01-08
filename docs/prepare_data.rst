Prepare Network Data
====================
The Tripal Network module is designed to support gene co-expression networks (GCNs), protein-protein interaction networks (PPIs), and top-down correlative metabolic networks. To load a network you must have the following data:

- Gene Co-expression Networks or Protein-Protein Interaction Networks:

  - Required

    - **The gene, mRNA (transcript) or protein features**. You can load genes, mRNA and proteins using the built-in Tripal GFF3 and FASTA loaders.
    - **The network graph**. The network graph is imported into Tripal using a GraphML loader provided by the Tripal Network module. All networks must first be converted into a GraphML format.

  - Optional

    - **Function annotations** about each gene or mRNA.  Often users want to explore the functions of important nodes in the network. This often includes Gene Ontology (GO) annotations, Protein domains and families (perhaps from InterProScan) and perhaps KEGG pathways and ortholog assignments. The Tripal network module will make available any functional annotation in the 3D viewer.  Sometimes annotations can be included in the GFF3 file but often they are not. The Tripal Network module provides a Chado Bulk Loader template for importing functional data if it is not already present in the database.


.. warning::

    Support for top-down metabolic networks is still under development.  Check back for updates with a future release of the module.

Preparing Genomic Data
----------------------
Prior to importing a network you must first import the genes, mRNA or protein features that will be used as nodes in the network.  This is often done using the GFF3 or FASTA loaders of Tripal. `Instructions for using these importers <https://tripal.readthedocs.io/en/latest/user_guide/example_genomics/genomes_genes.html>`_ are available in the `Tripal User's Guide <https://tripal.readthedocs.io/en/latest/index.html>`_.

Preparing Functional Data
-------------------------
If functional data in the form of controlled vocabulary terms, for example Gene Ontology terms, are already assigned to the genes, transcripts or proteins used in the networks then you can skip this step. However, if you need to add these data, you can do so using the Tripal `Chado Bulk Loader <https://tripal.readthedocs.io/en/latest/user_guide/bulk_loader.html>`_. This loader allows for import of any tab-delimited file provided a template for it exists.  The Tripal Network module provides a template for importing functional annotations in a tab-delimited file with rows of data ordered in the following columns:

1. Gene, mRNA or protein or protein name. This corresponds to the value in the Chado ``feature`` table ``name`` column
2. Term accession (e.g. "GO:0004396"). This value should include the vocabulary short name (e.g. "GO"), separated by a colon and the accession number.
3. Term name (e.g. "hexokinase activity").
4. Controlled Vocabulary (CV) name (e.g. "molecular_function").
5. Database name (e.g. "GO").  The database name is often the same as the short name of the vocabulary, but, it can be similar to the CV name or it may be different. If you're unsure what value to use here please ask for help on the `Tripal Slack workspace <http://tripal.info/community>`_.
6. Term definition (e.g. "Catalysis of the reaction: ATP + D-hexose = ADP + D-hexose 6-phosphate. Source: EC:2.7.1.1")

Some other requirements
- The file must be tab-delimited.
- Quotations are not needed to separate values in columns.
- A one-line header line is supported.
- The features in the file must all belong to the same organism.

Below is an example of a compatible file for a few  `Oryza sativa` (rice) genes.

.. code::

    Feature Term    Name    CV      DB      Definition
    LOC_Os06g41050  GO:0000014      single-stranded DNA endodeoxyribonuclease activity      molecular_function      GO      Catalysis of the hydrolysis of ester linkages within a single-stranded deoxyribonucleic acid molecule by creating internal breaks.
    LOC_Os01g60660  GO:0000049      tRNA binding    molecular_function      GO      Binding to a transfer RNA.
    LOC_Os02g46130  GO:0000166      nucleotide binding      molecular_function      GO      Binding to a nucleotide, any compound consisting of a nucleoside that is esterified with (ortho)phosphate or an oligophosphate at any hydroxyl group on the ribose or deoxyribose.


Generating a GraphML file
-------------------------
`GraphML <http://graphml.graphdrawing.org/>`_ is an XML format for storing network graphs. The Tripal Network module currently only supports import of networks in this format.  Below are a few instructions for generating GraphML files

Using Cytoscape
+++++++++++++++
`Cytoscape <https://cytoscape.org/>`_ is one of the most popular network visualization tools. You can import networks and visualize them in a 2D space.  You can create a GraphML file using Cytoscape by first loading a network and then `exporting it in GraphML format <https://manual.cytoscape.org/en/stable/Export_Your_Data.html>`_.

From a Tab Delimited File
+++++++++++++++++++++++++
Currently, there is not a loader for networks in a tab delimited format.  Use Cytoscape to first import the network in the tab delimited file then export to GraphML.

From WGCNA Networks
+++++++++++++++++++
The `Weighted Gene Co-expression Network Analysis (WGCNA) <https://horvath.genetics.ucla.edu/html/CoexpressionNetwork/Rpackages/WGCNA/>`_ software is one of the most popular tools for creating networks.  WGCNA has been cited in thousands of publications that incorporate networks into their analysis.  One of the major benefits of WGCNA is that it can circumscribe genes into modules of highly interacting genes. Those modules can then be associated with clinical or physiological traits and experimental conditions.  Often it is these modules that are reported but the underlying network is not. It is possible to extract the network from WGCNA, rather than just the module list, but the exact threshold to use is not precise.


Below is example R code that outputs a graphML file using functions from WGCNA and the `igraph R library <https://igraph.org/r/>`_:

.. code:: R

    # Import igraph
    library(igraph)
    library(WGCNA)

    # Set the network threshold. This will affect the size of the network.
    # You may need to adjust this value to generate the desired size network.
    hard_threshold = 0.082

    # Example code for running WGCNA (fill in appropriate arguments in place of the ...)
    net = blockwiseModules(gemt, ..., saveTOMs = TRUE, saveTOMFileBase = "TOM")
    blocks = sort(unique(net$blocks))

    # Stores the network edges in an edges array.
    edges = data.frame(fromNode= c(), toNode=c(), weight=c(), direction=c(), fromAltName=c(), toAltName=c())

    # iterate through each block, load the TOM file and convert the
    # network
    for (i in blocks) {
      # Load the TOM from a file.
      load(net$TOMFiles[i])
      TOM_size = length(which(net$blocks == i))
      TOM = as.matrix(TOM, nrow=TOM_size, ncol=TOM_size)
      colnames(TOM) = colnames(gemt)[net$blockGenes[[i]]]
      row.names(TOM) = colnames(gemt)[net$blockGenes[[i]]]

      cydata = exportNetworkToCytoscape(TOM, threshold = hard_threshold)
      edges = rbind(edges, cydata$edgeData)
    }

    # Set the interaction to 'co' for co-expression or correlation and
    # rename the columns to be more intuitive.
    edges$Interaction = 'co'
    output = edges[,c('fromNode','toNode','Interaction', 'weight')]
    colnames(output) = c('Source', 'Target', 'Interaction', 'WGCNA_weight')

    # Now save the edges as a graphML file.
    g = graph_from_data_frame(edges, directed = FALSE)
    write_graph(g, opt$network_graphml_file, 'graphml')


From KINC Networks
++++++++++++++++++
The `Knowledge Independent Network Construction (KINC) <https://kinc.readthedocs.io/en/latest/>`_ toolkit was developed by the `Ficklin <http://ficklinlab.cahnrs.wsu.edu/>`_ and  `Feltus Labs <https://scienceweb.clemson.edu/chg/f-alex-feltus/>`_ at Washington State University and Clemson University respectively.  KINC is C++, Phyhon and R suite of tools used to construct Condition-Specific Gene Co-expression Networks (csGCNs). However, it can also be used for top-down metabolic network construction as well.  One of the tools that it provides is a script named ``kinc-net2graphml.py`` Python script which can be used for exporting a KINC network into GraphML format compatible with the Tripal Network module.
