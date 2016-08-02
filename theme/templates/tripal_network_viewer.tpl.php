<?php
$module_path = drupal_get_path('module', 'tripal_network');
?>

<div id="graph-container">

</div>
<span id="layout-notification"></span>

<div id="tripal-network-viewer-settings-panel">
  <div class="toggle-header">
    <img class="toggle-icon" src="<?php print $module_path . '/theme/images/menu_icon.png'?>">
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
        <label>Select Node</label>
        <select id="nodelist">
          <option value="" selected>All nodes</option>
        </select>
      </div>
      <div class="form-item">
        <button id="reset-btn" class="pure-button pure-button-primary">Reset view</button>
      </div>
    </div>
  </div>
</div>


<div id="tripal-network-viewer-filter-panel">
  <div class="toggle-header">
    <img class="toggle-icon" src="<?php print $module_path . '/theme/images/menu_icon.png'?>">
    <h2>Filters</h2>
  </div>
  <div class="toggle-content"><?php
    // Add in the filter form.
    module_load_include('inc', 'tripal_network', 'includes/tripal_network.viewer');
    $form =  drupal_get_form('tripal_network_viewer_filter_form');
    print(drupal_render($form)); ?>
  </div>
</div>


<div id="tripal-network-viewer-data-panel">
  <div class="toggle-header">
    <img class="toggle-icon" src="<?php print $module_path . '/theme/images/menu_icon.png'?>">
    <h2>Data</h2>
  </div>
  <div class="toggle-content">
    <ul>
      <li><a id="network_info" href="#data-panel-network-stats">Network</a></li>
      <li><a id="edges_info" href="#data-panel-edge-list">Edges</a></li>
      <li><a id="nodes_info" href="#data-panel-node-list">Nodes</a></li>
      <li><a id="markers_info" href="#data-panel-markers-list">Markers</a></li>
      <li><a id="functional_info" href="#data-panel-functional-list">Functional</a></li>
      <li><a  href="#data-panel-current-list">Current Selection</a></li>
    </ul>
    <div id="data-panel-network-stats" class="data-panel-item">
    Network stats go here.
    </div>
    <div id="data-panel-edge-list" class="data-panel-item">
       <table id="data-panel-edge-list-table" width='100%' cellspacing='0'>
        <thead>
          <tr>
            <th>Source </th>
            <th>Target</th>
            <th>Weight</th>
          </tr>
        </thead>
        <tbody>
        </tbody>
       </table>
     </div>

    <div id="data-panel-node-list">
      <table id="data-panel-node-list-table" width='100%' cellspacing='0'>
        <thead>
          <tr>
            <th>Node Name</th>
            <th>Function Annotations</th>
          </tr>
        </thead>
        <tbody>
        </tbody>
      </table>

    </div>

    <div id="data-panel-markers-list">
    </div>

    <div id="data-panel-functional-list">
    </div>

    <div id="data-panel-current-list">
       
    </div>

  </div>
</div>