<?php
global $user;

$site_name = variable_get('site_name');

$theme_path = url(drupal_get_path('module', 'tripal_network') . '/theme', array('absolute' => TRUE));
$js_path = url(drupal_get_path('module', 'tripal_network') . '/theme/js', array('absolute' => TRUE));
$css_path = url(drupal_get_path('module', 'tripal_network') . '/theme/css', array('absolute' => TRUE));
drupal_add_js('https://cdn.plot.ly/plotly-2.3.1.min.js');
drupal_add_js('https://cdnjs.cloudflare.com/ajax/libs/d3/3.5.17/d3.min.js');
drupal_add_js($js_path . '/tripal_network_viewer.js');
drupal_add_css($css_path . '/tripal_network_viewer.css', 'external');

$args = [];
if ($organism_id) {
  $args['organism_id'] = $oragnism_id;
  if ($network_id) {
    $args['network_id'] = $network_id;
  }
}

drupal_add_js("
  (function(\$) {
    $(document).ready(function() {
      \$.fn.initViewer(" . json_encode($args) . ");
      \$.fn.getNetwork({});
    });
  })(jQuery);
", 'inline');


$network_form = drupal_get_form('tripal_network_viewer_network_form', $organism_id, $network_id);
$network_form = drupal_render($network_form);

$layers_form = drupal_get_form('tripal_network_viewer_layers_form', $organism_id, $network_id);
$layers_form = drupal_render($layers_form);

$filter_form = drupal_get_form('tripal_network_viewer_filter_form', $organism_id, $network_id);
$filter_form = drupal_render($filter_form);

$node_details = drupal_get_form('tripal_network_viewer_node_details_form');
$node_details = drupal_render($node_details);

$edge_details = drupal_get_form('tripal_network_viewer_edge_details_form');
$edge_details = drupal_render($edge_details);

?>
<div id="tripal-network-viewer-app">
   <div id="tripal-network-viewer-loading"><img src="<?php print $theme_path?>/images/loading.gif"></div>
   <div id="tripal-network-viewer-navbar">
     <div><img id="tripal-network-viewer-network-icon" class="tripal-network-viewer-navbar-icon" src="<?php print $theme_path?>/images/noun_Network_1571006.png"></div>
     <div><img id="tripal-network-viewer-layers-icon" class="tripal-network-viewer-navbar-icon" src="<?php print $theme_path?>/images/noun_Layers_4177660.png"></div>
     <div><img id="tripal-network-viewer-filters-icon" class="tripal-network-viewer-navbar-icon" src="<?php print $theme_path?>/images/noun_filter_4186018.png"></div>
     <div><img id="tripal-network-viewer-node-icon" class="tripal-network-viewer-navbar-icon" src="<?php print $theme_path?>/images/noun_Circle_3927915.png"></div>
     <div><img id="tripal-network-viewer-edge-icon" class="tripal-network-viewer-navbar-icon" src="<?php print $theme_path?>/images/noun_link_2545631.png"></div>
     <div><img id="tripal-network-viewer-about-icon" class="tripal-network-viewer-navbar-icon" src="<?php print $theme_path?>/images/noun_about_4190914.png"></div>
     <div><img id="tripal-network-viewer-help-icon" class="tripal-network-viewer-navbar-icon" src="<?php print $theme_path?>/images/noun_help_2302539.png"></div>
   </div>
   <div id="tripal-network-viewer-sidebar">
     <div id="tripal-network-viewer-sidebar-toggle">
       <img id="tripal-network-viewer-slide-icon" class="tripal-network-viewer-sidebar-icon" src="<?php print $theme_path?>/images/noun_slide_3149648.png">
     </div>
     <div id="tripal-network-viewer-sidebar-boxes">
       <div id="tripal-network-viewer-sidebar-header">
         <h1>Network Viewer</h1>
         <?php print l('Site Home', '/')?>
       </div>
       <div id="tripal-network-viewer-network-box" class="tripal-network-viewer-sidebar-box">
    	   <h2>Network Selection</h2>
       	 <div class="tripal-network-viewer-sidebar-box-content">
       	   <?php print $network_form ?>
       	 </div>
       </div>
       <div id="tripal-network-viewer-layers-box" class="tripal-network-viewer-sidebar-box">
         	<h2>Layers and Colors</h2>
           <?php print $layers_form ?>
       </div>
       <div id="tripal-network-viewer-filters-box" class="tripal-network-viewer-sidebar-box">
         	<h2>Filters</h2>
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
                <li>Network icon by Three Six Five from the Noun Project</li>
                <li>Layers icon by Thomas from the Noun Project</li>
                <li>Filter icon by Adam Baihaqi from the Noun Project</li>
                <li>Circle by Lars Meiertoberens from the Noun Project</li>
                <li>Link icon by Alex Burte from the Noun Project</li>
                <li>About icon by andika from the Noun Project</li>
                <li>Slide icon by Alebaer from the Noun Project</li>
                <li>Help icon by Rainbow Designs from the Noun Project</li>
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
