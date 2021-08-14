 <?php
 if ($messages) { ?>
 <div id="messages">
   <div class="section clearfix">
     <?php print $messages; ?>
   </div>
 </div><?php
}
else {
  print drupal_render($page['content']);
}?>

