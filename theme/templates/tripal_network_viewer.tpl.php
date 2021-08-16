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

if ($network_id) {

  // Load the network that was provided if the user has access.
  $entity_id = chado_get_record_entity_by_table('network', $network_id);
  if ($entity_id){
    $entity = tripal_load_entity('TripalEntity', [$entity_id]);

    if (tripal_entity_access('view', $entity[$entity_id], $user, 'TripalEntity')) {
      drupal_add_js("(function(\$) { $(document).ready(function() { \$.fn.getNetwork({'network_id': $network_id }); }); })(jQuery);", 'inline');
    }
    else {
      drupal_set_message(t('You must be granted permission to view this network.'), 'error');
    }
  }
  else {
    drupal_set_message(t('The requested network does not exist.'), 'error');
  }
}

$network_form = drupal_get_form('tripal_network_viewer_network_form', $network_id);
$network_form = drupal_render($network_form);

$display_form = drupal_get_form('tripal_network_viewer_display_form', $network_id);
$display_form = drupal_render($display_form);

$network_details = drupal_get_form('tripal_network_viewer_network_details_form', $network_id);
$network_details = drupal_render($network_details);

$node_details = drupal_get_form('tripal_network_viewer_node_details_form');
$node_details = drupal_render($node_details);

$edge_details = drupal_get_form('tripal_network_viewer_edge_details_form');
$edge_details = drupal_render($edge_details);

?>
<div id="tripal-network-viewer-app">
   <div id="tripal-network-viewer-loading"><img src="<?php print $theme_path?>/images/loading.gif"></div>
   <div id="tripal-network-viewer-sidebar">
     <?php print l('Site Home', '/')?>
     <h1><?php print $site_name ?> Network Viewer</h1>
     <div id="tripal-network-viewer-network-select" class="tripal-network-viewer-sidebar-box">
     	 <div class="tripal-network-viewer-sidebar-box-header">
     	   <img class="tripal-network-viewer-sidebar-box-header-toggle-on" src="<?php print $theme_path?>/images/toggle-on.png">
         <img class="tripal-network-viewer-sidebar-box-header-toggle-off" src="<?php print $theme_path?>/images/toggle-off.png">
     	   <h2>Network Selection</h2>
     	 </div>
     	 <div class="tripal-network-viewer-sidebar-box-content">
     	   <?php print $network_form ?>
     	 </div>
     </div>
     <div id="tripal-network-viewer-display-details" class="tripal-network-viewer-sidebar-box">
        <div class="tripal-network-viewer-sidebar-box-header">
        	<img class="tripal-network-viewer-sidebar-box-header-toggle-on" src="<?php print $theme_path?>/images/toggle-on.png">
        	<img class="tripal-network-viewer-sidebar-box-header-toggle-off" src="<?php print $theme_path?>/images/toggle-off.png">
        	<h2>Display</h2>
        </div>
        <div class="tripal-network-viewer-sidebar-box-content">
          <?php print $display_form ?>
        </div>
     </div>
     <div id="tripal-network-viewer-network-details" class="tripal-network-viewer-sidebar-box">
        <div class="tripal-network-viewer-sidebar-box-header">
          <img class="tripal-network-viewer-sidebar-box-header-toggle-on" src="<?php print $theme_path?>/images/toggle-on.png">
          <img class="tripal-network-viewer-sidebar-box-header-toggle-off" src="<?php print $theme_path?>/images/toggle-off.png">
           	<h2>Network</h2>
      	</div>
        <div class="tripal-network-viewer-sidebar-box-content">
        <?php print $network_details ?>
        </div>
     </div>
     <div id="tripal-network-viewer-node-details" class="tripal-network-viewer-sidebar-box">
        <div class="tripal-network-viewer-sidebar-box-header">
          <img class="tripal-network-viewer-sidebar-box-header-toggle-on" src="<?php print $theme_path?>/images/toggle-on.png">
          <img class="tripal-network-viewer-sidebar-box-header-toggle-off" src="<?php print $theme_path?>/images/toggle-off.png">
        	<h2>Node</h2>
        </div>
        <div class="tripal-network-viewer-sidebar-box-content">
          <?php print $node_details ?>
        </div>
     </div>
     <div id="tripal-network-viewer-edge-details" class="tripal-network-viewer-sidebar-box">
        <div class="tripal-network-viewer-sidebar-box-header">
          <img class="tripal-network-viewer-sidebar-box-header-toggle-on" src="<?php print $theme_path?>/images/toggle-on.png">
          <img class="tripal-network-viewer-sidebar-box-header-toggle-off" src="<?php print $theme_path?>/images/toggle-off.png">
        	<h2>Edge</h2>
      </div>
        <div class="tripal-network-viewer-sidebar-box-content">
        <?php print $edge_details ?>
        </div>
     </div>
   </div>
   <div id="tripal-network-viewer-display">
     <div id="tripal-network-viewer"></div>
   </div>
   <div id="tripal-network-viewer-footer">
      <div class="tripal-network-viewer-footer-item-left">
        <br>This viewer is provided by the <br><?php print l('Tripal Network Module', 'https://github.com/tripal/tripal_network')?>
      </div>
      <div class="tripal-network-viewer-footer-item tripal-network-viewer-footer-item-right">
        <img src="<?php print $theme_path?>/images/powered_by_tripal_small.png"><br>
        Powered by <?php print l('Tripal', 'https://tripal.info')?>
      </div>
      <div class="tripal-network-viewer-footer-item tripal-network-viewer-footer-item-right">
        <img src="<?php print $theme_path?>/images/inspired_by_KINC_small.png"><br>
         Inspired by <?php print l('KINC', 'https://kinc.readthedocs.io/')?>
      </div>
   </div>
</div>
