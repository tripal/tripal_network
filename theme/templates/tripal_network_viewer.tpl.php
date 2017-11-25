<?php
$module_path = drupal_get_path('module', 'tripal_network');
?>

<div id="graph-container">

</div>
<span id="layout-notification"></span>

<div id="tripal-network-viewer-settings-panel">
  <div class="toggle-header">
    <img class="toggle-icon" src="<?php print url($module_path . '/theme/images/menu_icon.png')?>">
    <h2>Display</h2>
  </div>
  <div class="toggle-content">
    <div id="bg-color-picker">
      <h4>Background Color</h4>
      <div class="bg-color-picker" style="background-color: #FFFFFF">&nbsp</div>
      <div class="bg-color-picker" style="background-color: #000044">&nbsp</div>
      <div class="bg-color-picker" style="background-color: #666666">&nbsp</div>
      <div class="bg-color-picker" style="background-color: #444444">&nbsp</div>
      <div class="bg-color-picker" style="background-color: #000000">&nbsp</div>
    </div>
    <div id="select-form">
      <div class="form-item">
        <button id="reset-btn" class="pure-button pure-button-primary">Reset view</button>
      </div>
    </div>
  </div>
</div>


<div id="tripal-network-viewer-filter-panel">
  <div class="toggle-header">
    <img class="toggle-icon" src="<?php print url($module_path . '/theme/images/menu_icon.png')?>">
    <h2>Filters</h2>
  </div>
  <div class="toggle-content"><?php
    // Add in the filter form.
    module_load_include('inc', 'tripal_network', 'includes/tripal_network.viewer');
    $form =  drupal_get_form('tripal_network_viewer_filter_form');
    print(drupal_render($form)); ?>
  </div>
</div>