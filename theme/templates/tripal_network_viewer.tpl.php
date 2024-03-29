<?php
global $user;
$network_session_id = NULL;
$organism_id = array_key_exists('organism_id', $_GET) ? $_GET['organism_id'] : NULL;
$network_id = array_key_exists('network_id', $_GET) ? $_GET['network_id'] : NULL;
$feature_id = array_key_exists('feature_id', $_GET) ? $_GET['feature_id'] : NULL;

$site_name = variable_get('site_name');

$theme_path = url(drupal_get_path('module', 'tripal_network') . '/theme', array('absolute' => TRUE));
$js_path = url(drupal_get_path('module', 'tripal_network') . '/theme/js', array('absolute' => TRUE));
$css_path = url(drupal_get_path('module', 'tripal_network') . '/theme/css', array('absolute' => TRUE));
drupal_add_js('https://cdn.plot.ly/plotly-2.3.1.min.js');
drupal_add_js('https://cdnjs.cloudflare.com/ajax/libs/d3/3.5.17/d3.min.js');
drupal_add_js($js_path . '/tripal_network_viewer.js');
drupal_add_css($css_path . '/tripal_network_viewer.css', 'external');
drupal_add_js('misc/form.js');
drupal_add_js('misc/collapse.js');
drupal_add_js("https://code.jquery.com/ui/1.13.1/jquery-ui.js");
drupal_add_css("https://code.jquery.com/ui/1.13.1/themes/base/jquery-ui.css", 'external');


// We want to suppor tthe analysis expression bargraph field but that gets loaded by AJAX
// so we need to maake sure it's JS is present.
$tae_js_path = url(drupal_get_path('module', 'tripal_analysis_expression') . '/theme/js', array('absolute' => TRUE));
$tae_css_path = url(drupal_get_path('module', 'tripal_analysis_expression') . '/theme/css', array('absolute' => TRUE));
//drupal_add_js($tae_js_path . '/expression.js');
//drupal_add_css($tae_css_path . '/expression.css');

$args = [];
if ($organism_id) {
  $args['organism_id'] = $organism_id;
}
if ($network_id) {
  $args['network_id'] = $network_id;
}
if ($feature_id) {
  $args['feature_id'] = $feature_id;
}

drupal_add_js("
  (function(\$) {
    $(document).ready(function() {
      \$.fn.initViewer(" . json_encode($args) . ");
    });
  })(jQuery);
", 'inline');


$network_form = drupal_get_form('tripal_network_viewer_network_form', $organism_id, $network_id, $feature_id);
$network_form = drupal_render($network_form);

$layers_form = drupal_get_form('tripal_network_viewer_layers_form', $network_session_id, $organism_id, $network_id);
$layers_form = drupal_render($layers_form);

$filter_form = drupal_get_form('tripal_network_viewer_filter_form', $network_session_id, $organism_id, $network_id);
$filter_form = drupal_render($filter_form);

$network_details = drupal_get_form('tripal_network_viewer_network_details_form');
$network_details = drupal_render($network_details);

$node_details = drupal_get_form('tripal_network_viewer_node_details_form');
$node_details = drupal_render($node_details);

$edge_details = drupal_get_form('tripal_network_viewer_edge_details_form');
$edge_details = drupal_render($edge_details);

$edge_data_form = drupal_get_form('tripal_network_viewer_edge_data_form', $network_session_id, $organism_id, $network_id);
$edge_data_form = drupal_render($edge_data_form);

$node_data_form = drupal_get_form('tripal_network_viewer_node_data_form', $network_session_id, $organism_id, $network_id);
$node_data_form = drupal_render($node_data_form);

$analysis_form = drupal_get_form('tripal_network_viewer_analysis_form', $network_session_id, $organism_id, $network_id);
$analysis_form = drupal_render($analysis_form);

?>
<div id="tripal-network-viewer-app">
   <div id="tripal-network-viewer-loading">
   	 <div id="tripal-network-viewer-loading-spinner"></div>
   </div>
   <ul id="tripal-network-viewer-node-menu">
      <li class="ui-widget-header"><div id="node-menu-header">Node Menu</div></li>
      <li><div id="node-select"><span class="ui-icon ui-icon-cart"></span>Select</div></li>
      <li><div id="node-select-neighbors"><span class="ui-icon ui-icon-cart"></span>Select Neighbors</div></li>
      <li><div id="node-inspect"><span class="ui-icon ui-icon-document"></span>Inspect</div></li>
   </ul>
   <div id="tripal-network-viewer-navbar">
     <div><img id="tripal-network-viewer-network-select-icon" class="tripal-network-viewer-navbar-icon" src="<?php print $theme_path?>/images/noun_selection_2256434.png"></div>
     <div><img id="tripal-network-viewer-network-details-icon" class="tripal-network-viewer-navbar-icon" src="<?php print $theme_path?>/images/noun_Network_1571006.png"></div>
     <div><img id="tripal-network-viewer-layers-icon" class="tripal-network-viewer-navbar-icon" src="<?php print $theme_path?>/images/noun_Layers_4177660.png"></div>
     <div><img id="tripal-network-viewer-filters-icon" class="tripal-network-viewer-navbar-icon" src="<?php print $theme_path?>/images/noun_filter_4186018.png"></div>
     <div><img id="tripal-network-viewer-node-icon" class="tripal-network-viewer-navbar-icon" src="<?php print $theme_path?>/images/noun_Circle_3927915.png"></div>
     <div><img id="tripal-network-viewer-edge-icon" class="tripal-network-viewer-navbar-icon" src="<?php print $theme_path?>/images/noun_link_2545631.png"></div>
     <div><img id="tripal-network-viewer-data-icon" class="tripal-network-viewer-navbar-icon" src="<?php print $theme_path?>/images/noun_table_1061423.png"></div>
     <div><img id="tripal-network-viewer-analysis-icon" class="tripal-network-viewer-navbar-icon" src="<?php print $theme_path?>/images/noun_analysis_2018398.png"></div>
     <div><img id="tripal-network-viewer-about-icon" class="tripal-network-viewer-navbar-icon" src="<?php print $theme_path?>/images/noun_about_4190914.png"></div>
     <div><img id="tripal-network-viewer-help-icon" class="tripal-network-viewer-navbar-icon" src="<?php print $theme_path?>/images/noun_help_2302539.png"></div>
   </div>
   <div id="tripal-network-viewer-sidebar-toggle">
     <div id ="tripal-network-viewer-slide-icons">
       <img id="tripal-network-viewer-slide-right-icon" class="tripal-network-viewer-sidebar-icon" src="<?php print $theme_path?>/images/noun-fast-forward-57089.png">
       <img id="tripal-network-viewer-slide-left-icon" class="tripal-network-viewer-sidebar-icon" src="<?php print $theme_path?>/images/noun-rewind-57114.png">
     </div>
   </div>
   <div id="tripal-network-viewer-sidebar">
     <div id="tripal-network-viewer-sidebar-boxes">
       <div id="tripal-network-viewer-sidebar-header">
         <h1>3D Network Explorer</h1>
         <?php print l('Site Home', '/')?>
       </div>
       <div id="tripal-network-viewer-network-select-box" class="tripal-network-viewer-sidebar-box">
    	   <h2>Network Selection</h2>
       	 <div class="tripal-network-viewer-sidebar-box-content">
       	   <?php print $network_form ?>
       	 </div>
       </div>
       <div id="tripal-network-viewer-network-details-box" class="tripal-network-viewer-sidebar-box">
    	   <h2>Network Details</h2>
       	 <div class="tripal-network-viewer-sidebar-box-content">
       	   <?php print $network_details ?>
       	 </div>
       </div>
       <div id="tripal-network-viewer-layers-box" class="tripal-network-viewer-sidebar-box">
         	<h2>Layers and Colors</h2>
           <?php print $layers_form ?>
       </div>
       <div id="tripal-network-viewer-filters-box" class="tripal-network-viewer-sidebar-box">
         	<h2>Node/Edge Property Filters</h2>
           <?php print $filter_form ?>
       </div>
       <div id="tripal-network-viewer-node-box" class="tripal-network-viewer-sidebar-box">
        	<h2>Selected Node Details</h2>
          <?php print $node_details ?>
       </div>
       <div id="tripal-network-viewer-edge-box" class="tripal-network-viewer-sidebar-box">
         	<h2>Selected Edge Details</h2>
          <?php print $edge_details ?>
       </div>
       <div id="tripal-network-viewer-data-box" class="tripal-network-viewer-sidebar-box">
         	<h2>Data Tables</h2>
          <div id="tripal-network-viewer-data-tabs">
            <ul> 
              <li><a href="#tripal-network-viewer-edge-data-tab">Edges</a></li>
              <li><a href="#tripal-network-viewer-node-data-tab">Nodes</a></li>
            </ul>
            <div id="tripal-network-viewer-edge-data-tab">
            	<?php print $edge_data_form ?>          		
            </div>
            <div id="tripal-network-viewer-node-data-tab">
            	<?php print $node_data_form ?>
            </div>
          </div>         	 
       </div>
       <div id="tripal-network-viewer-analysis-box" class="tripal-network-viewer-sidebar-box">
         	<h2>Analysis</h2>   
         	<div id="tripal-network-viewer-analysis-tab">
         	  <?php print $analysis_form ?>
         	</div>
       </div>
       <div id="tripal-network-viewer-about-box" class="tripal-network-viewer-sidebar-box">
          <h2>About this Network Viewer</h2>
          <div class="tripal-network-viewer-sidebar-box-content">
            <p>The Tripal Network Viewer was created by the <a href="http://ficklinlab.cahnrs.wsu.edu">Ficklin
              Computational Biology Team</a> at <a href="http://www.wsu.edu">Washington State University</a
              and is an open-source plugin for Tripal-based sites</p>
            <p>Powered by <?php print l('Tripal', 'https://tripal.info')?><br>
            <img src="<?php print $theme_path?>/images/powered_by_tripal_small.png"></p>
            <p>Inspired by <?php print l('KINC', 'https://kinc.readthedocs.io/')?><br>
            <img src="<?php print $theme_path?>/images/inspired_by_KINC_small.png"></p>
            <p>
              <b>Icons Attribution</b>
              <ul>
                <li>Network selection icon by Lauk from the NounProject.com</li>
                <li>Network details icon by Markus from the NounProject.com</li>
                <li>Layers icon by Thomas from the NounProject.com</li>
                <li>Filter icon by Adam Baihaqi from the NounProject.com</li>
                <li>Circle by Lars Meiertoberens from the NounProject.com</li>
                <li>Link icon by Alex Burte from the NounProject.com</li>
                <li>Analysis icon by hashank singh from the NounProject.com</li> 
                <li>About icon by andika from the NounProject.com</li>
                <li>Slide icon by Alebaer from the NounProject.com</li>
                <li>Help icon by Rainbow Designs from the NounProject.com</li>
              </ul>
            </p>
          </div>
       </div>
       <div id="tripal-network-viewer-help-box" class="tripal-network-viewer-sidebar-box">
         <h2>Help</h2>
         <div class="tripal-network-viewer-sidebar-box-content">
         </div>
       </div>
     </div>

   </div>
   <div id="tripal-network-viewer-display"> 	 
     <div id="tripal-network-viewer"></div>
   </div>
</div>
