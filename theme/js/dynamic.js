console.log("Into the laast script");
var $j = jQuery.noConflict();
$j(document).ready(function(){
    $j("#filter").click(function(){
      console.log("Filter is clicked");
        $j("#dataform").slideToggle("fast");
    });
});


$j(document).ready(function(){
    $j("#get_table").click(function(){
        $j("#dataset").slideToggle("fast");
    });
});

 console.log("Slide Toggle is done");
  $j(function() {
    $j( "#dataform" ).draggable();
    console.log("Draggable is enabled");
    $j("#dataset").draggable();
    $j(".table1").resizable();
    $j("#control-pane").draggable();

  });