<?php
/**
 * @file
 * @todo Add file header description
 */

/**
 * @defgroup graphML_loader graphML Feature Loader
 * @{
 * Provides graphML loading functionality. Creates features based on their specification in a graphML file.
 * @}
 * @ingroup tripal_network
 */

/**
 * Creates the frm for the GraphML loader page.
 *
 * @return
 *   A Drupal Form API compatible array.
 */
function tripal_network_graphML_load_form($form, $form_state) {

  $form['graphML_file']= array(
    '#type'          => 'textfield',
    '#title'         => t('graphML File'),
    '#description'   => t('Please enter the full system path for the gramML
      (GML) file, or a path within the Drupal installation (e.g.
      /sites/default/files/xyz.gml).  The path must be accessible to the
      server on which this Drupal instance is running.'),
    '#required' => TRUE,
  );

  $organisms = tripal_get_organism_select_options(FALSE);
  $form['organism_id'] = array(
    '#title'       => t('Organism'),
    '#type'        => t('select'),
    '#description' => t("Choose the organism to which these sequences are associated"),
    '#required'    => TRUE,
    '#options'     => $organisms,
  );

  $form['analysis'] = array(
    '#type' => 'fieldset',
    '#title' => t('Analysis Used to Derive the Network'),
    '#collapsed' => TRUE
  );
  $form['analysis']['desc'] = array(
    '#type' => 'markup',
    '#value' => t("Why specify an analysis for a data load?  All data comes
       from some place, even if downloaded from somewhere. By specifying
       analysis details for all data uploads, it allows an end user to reproduce the
       data set, but at least indicates the source of the data."),
  );

  // TODO: we need more fields to help the user specify the unique constraint
  // for matching features.

  $analyses = tripal_get_analysis_select_options(FALSE);
  $form['analysis']['analysis_id'] = array(
   '#title'       => t('Analysis'),
   '#type'        => t('select'),
   '#description' => t("Choose the analysis to which this graph is associated"),
   '#required'    => TRUE,
   '#options'     => $analyses,
  );

  $form['button'] = array(
    '#type' => 'submit',
    '#value' => t('Import graphML file'),
  );

  return $form;
}

/**
 * The validate function for the tripal_network_graphML_load_form form.
 *
 * @param $form
 *   The Drupal form array.
 * @param $form_state
 *   The Drupal form_state array.
 */
function tripal_network_graphML_load_form_validate($form, &$form_state) {

  $graphML_file = $form_state['values']['graphML_file'];
  $organism_id = $form_state['values']['organism_id'];

  // check to see if the file is located local to Drupal
  $graphML_file = trim($graphML_file);
  $dfile = $_SERVER['DOCUMENT_ROOT'] . base_path() . $graphML_file;
  if (!file_exists($dfile)) {
    // if not local to Drupal, the file must be someplace else, just use
    // the full path provided
    $dfile = $graphML_file;
  }
  if (!file_exists($dfile)) {
    form_set_error('graphML_file', t("Cannot find the file on the system. Check that ".
      "the file exists or that the web server has permissions to read the file."));
  }
}

/**
 * The submit function for the tripal_network_graphML_load_form form.
 *
 * Adds a job to the Tripal Job system for importing the GraphML file.
 *
 * @param $form
 *   The Drupal form array.
 * @param $form_state
 *   The Drupal form_state array.
 */
function tripal_network_graphML_load_form_submit($form, &$form_state) {
  global $user;

  $graphML_file = $form_state['values']['graphML_file'];
  $organism_id = $form_state['values']['organism_id'];
  $analysis_id = $form_state['values']['analysis_id'];

  $args = array($graphML_file, $organism_id, $analysis_id);

  $fname = preg_replace("/.*\/(.*)/", "$1", $graphML_file);

  $includes = array(
    module_load_include('inc', 'tripal_network', 'includes/tripal_networ.grapphML_loade'),
  );
  tripal_add_job("Import graphML file: $fname", 'tripal_network',
    'tripal_network_load_graphML', $args, $user->uid, 10, $includes);

  return '';
}

/**
 * Imports the GraphML file containing a biological network.
 *
 * The nodes in the GraphML file should be the names of features in the
 * Chado feature table.
 *
 * @param $graphML_file
 *   The full path to the GraphML file on the file system.
 * @param $organism_id
 *   The ID of the organism to which this network belongs.
 * @param $analysis_id
 *   The ID of the analysis that was used to generate the network.
 * @param $job
 *   Optional. The ID of the job. This argument is provided by the Tripal
 *   Job system.
 *
 * @return
 *   TRUE on sucess, FALSE on failure.
 */
function tripal_network_load_graphML($graphML_file, $organism_id, $analysis_id, $job = NULL) {

  $species=$organism_id;

  //This will open the graphml file and put the cursor at the front
  $myfile=simplexml_load_file($graphML_file);

  //When file is not found. This has to be filled with a proper error alert instead of an echo

  if(!$myfile){
    echo "File cannot be loaded";
  }

  foreach($myfile->graph->node as $nodeinfo){
      $nodeid=$nodeinfo['id'];
      $nodeid=(string)$nodeid;
      // This statement is querying if the node with the given id exists;
      $var="MATCH(u:".$species.") WHERE u.id = {id} RETURN u.id";
       $data=array(
       "query" => $var,
        "params" => array(
          "id"=>$nodeid
        )
       );
      // This piece of code, will use the Neo4j REST API to pass the request
      $data=json_encode($data);  
      $curl = curl_init();
      curl_setopt($curl, CURLOPT_URL, 'http://localhost:7474/db/data/cypher/');
      curl_setopt($curl, CURLOPT_RETURNTRANSFER, 1);
      curl_setopt($curl,CURLOPT_HTTPHEADER,array('Accept: application/json; charset=UTF-8','Content-Type: application/json'));
      curl_setopt($curl, CURLOPT_CUSTOMREQUEST, "POST");                                                                     
      curl_setopt($curl, CURLOPT_POSTFIELDS,$data);

      //The result from the request gets stored in the $result1 variable

      $result1 = curl_exec($curl);

      //echo $result1;
      curl_close($curl); 
      //Converting the result into a json array
      $result=json_encode($result1);

      // The result given by the REST API request gives an empty field with "data". 
      $word="data";

      if(strpos($result,$word)!==false)
      {
        
        $pos=strpos($result,$word);
        
      }

      //Checking the position of "[" bracket. If two "[" with a space in middle exists continuously, then the node is already present
      $pos=$pos+11;

      //
      if($result[$pos]=="[")
      {
        echo "";
      }

      //Here in the else statement the query for the insertion of the nodes will be there.
      else{

        // Here query chado database with the nodeid to extract the Feature.type_id
        //
        $create_query="CREATE (x:".$species." { id : {id} }) RETURN x";
        $data=array(
         "query" => $create_query,
         "params" => array(
         "id" => $nodeid
        )
       ); 


      $data=json_encode($data);  
      $curl = curl_init();
      curl_setopt($curl, CURLOPT_URL, 'http://localhost:7474/db/data/cypher/');
      curl_setopt($curl, CURLOPT_RETURNTRANSFER, 1);
      curl_setopt($curl,CURLOPT_HTTPHEADER,array('Accept: application/json; charset=UTF-8','Content-Type: application/json'));
      curl_setopt($curl, CURLOPT_CUSTOMREQUEST, "POST");                                                                     
      curl_setopt($curl, CURLOPT_POSTFIELDS,$data);
      $result1 = curl_exec($curl);
      //echo $result1;
      curl_close($curl);

      }







  }

  //End of for loop for entering the required nodes


  //The following piece of code will scrape through the edges present and create relationships among the existing nodes

  foreach($myfile->graph->edge as $edgeinfo){
    //storing the module name

      $module = "";
      $networkname="";

      //For looping through data and finding appropriate keys 
      foreach($edgeinfo->children() as $dataset ){
        if($dataset['key']=='module'){
          $module = (string)$dataset;
        }
        else if($dataset['key']=='network'){
          $networkname = (string)$dataset;
        }
      }

      //$module=(string)$edgeinfo->data[0];

      //storing the network name
      //$networkname=(string)$edgeinfo->data[1];

      //storing the source
      $source=(string)$edgeinfo['source'];

      $target=(string)$edgeinfo['target'];

      //Naming the relationship 
      $module_rel = $species."module";
      $que = "MATCH(n1:".$species."{id:{id1}})-[rel:".$species."module]->(n2:".$species."{id:{id2}}) WHERE rel.modulename = {value} RETURN rel.modulename,n1.id,n2.id";

      //This query is checking if a relationship corresponding to the source and the destination with the specific label of the relationship already exists

       $data=array(
       "query" => $que,
        "params" => array(
          "id1"=>$source,
          "id2"=>$target,
          "value"=>$module
        )
       ); 


        $data=json_encode($data);  
        $curl = curl_init();
        curl_setopt($curl, CURLOPT_URL, 'http://localhost:7474/db/data/cypher/');
        curl_setopt($curl, CURLOPT_RETURNTRANSFER, 1);
        curl_setopt($curl,CURLOPT_HTTPHEADER,array('Accept: application/json; charset=UTF-8','Content-Type: application/json'));
        curl_setopt($curl, CURLOPT_CUSTOMREQUEST, "POST");                                                                     
        curl_setopt($curl, CURLOPT_POSTFIELDS,$data);
        $result1 = curl_exec($curl);

        //echo $result1;

        curl_close($curl);


        $result=json_encode($result1);
        $word="data";
        //Search for the word data in the string returned by the REST API call

        if(strpos($result,$word)!==false){
          
          $pos=strpos($result,$word);
          
        }

        //Checking the position of "[" bracket. If two "[" with a space in middle exists continuously, then the node is already present

        //Here there has to be an else statement which will handle any sort of errors
        $pos=$pos+11;


        // if "[" exists then that means there has been something that has been returned and a relationship of the desired type already exists

        if($result[$pos]=="["){
          echo "";
        }

        //
        else{
          
          //The following query will attach a relationship to two nodes with the source and target as extracted from the graphml file

            $insert_query="MATCH (u:".$species."{id:{source}}),(r:".$species."{id:{target}}) CREATE (u)-[:".$module_rel."{modulename:{mod},network:{net}}]->(r)";

            $data=array(
            "query" => $insert_query,
            "params" => array(
            "source" => $source,
            "target" =>$target,
            "mod"=>$module,
            "net"=>$networkname
          )
         ); 

            //encoding the data in json format
            $data=json_encode($data);  

            //Sending data through the Neo4j REST API to the server

            $curl = curl_init();
            curl_setopt($curl, CURLOPT_URL, 'http://localhost:7474/db/data/cypher/');
            curl_setopt($curl, CURLOPT_RETURNTRANSFER, 1);
            curl_setopt($curl,CURLOPT_HTTPHEADER,array('Accept: application/json; charset=UTF-8','Content-Type: application/json'));
            curl_setopt($curl, CURLOPT_CUSTOMREQUEST, "POST");                                                                     
            curl_setopt($curl, CURLOPT_POSTFIELDS,$data);
            $result1 = curl_exec($curl);
            //echo $result1;
            curl_close($curl);
        }







    }
}

/**
 * Imports the contents of the 'graph' section of the GraphML file.
 *
 * @param $graphml
 *   An XMLReader object of the 'graph' section.
 * @param $attributes
 *   An associative array containing the attributes of the graph.
 * @param $organism
 *   An organism object record.
 * @param $nodes_read
 *   A count of the number of nodes that have been read. This argument
 *   is passed by reference and is updated by this call.
 * @param $mod_type
 *   A cvterm object record representing the controlled vocabulary term
 *   for the network module type.
 */
