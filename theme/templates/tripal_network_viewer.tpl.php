<?php

$js_path = url(drupal_get_path('module', 'tripal_network') . '/theme/js', array('absolute' => TRUE));
$css_path = url(drupal_get_path('module', 'tripal_network') . '/theme/css', array('absolute' => TRUE));
drupal_add_js('https://cdn.plot.ly/plotly-2.3.0.min.js');
drupal_add_js('https://cdnjs.cloudflare.com/ajax/libs/d3/3.5.17/d3.min.js');
drupal_add_js($js_path . '/tripal_network_viewer.js');
drupal_add_css($css_path . '/tripal_network_viewer.css', 'external');

$viewer_id = 'tripal-network-viewer';
if ($network_id) {
  drupal_add_js("(function(\$) {\$.fn.getNetwork({'network_id': $network_id, 'viewer_id': '$viewer_id'}); })(jQuery);", 'inline');
}

$network_form = drupal_get_form('tripal_network_viewer_form', $network_id);
$network_form = drupal_render($network_form);

?>
<h1>Tripal Network Viewer</h1>
<div id="tripal-network-viewer-app">
   <?php print $network_form ?>
   <div id="tripal-network-viewer-sidebar">
     <div id="tripal-network-viewer-network-details" class="tripal-network-viewer-sidebar-box">
        <div class="tripal-network-viewer-sidebar-box-header">Network Details</div>
     </div>
     <div id="tripal-network-viewer-node-details" class="tripal-network-viewer-sidebar-box">
        <div class="tripal-network-viewer-sidebar-box-header">Node Details</div>
     </div>
     <div id="tripal-network-viewer-edge-details" class="tripal-network-viewer-sidebar-box">
        <div class="tripal-network-viewer-sidebar-box-header">Edge Details</div>
     </div>
   </div>
   <div id="tripal-network-viewer-display"><div id="tripal-network-viewer"></div></div>
</div>
