<script xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns="http://www.w3.org/1999/xhtml" type="text/javascript"><![CDATA[
  $(document).ready(function(){
    $("#edit").click(function(){
      switch ($(this).val()) {
        case "edit": {
          $(this).val("view");
          $(".Item, .draggable").draggable();
          $(".Item, .resizable").resizable();
          $(".Item, .zindexable").each(function(){
            $(this).prepend("<input class=\"z-index\" value=\"" + $(this).zIndex() + "\"/>")
          });
          $(".z-index").blur(function(){
            $(this).parent().zIndex($(this).val());
          });
          break;
        }
        default: {
          $(this).val("edit");

          var new_css = '';
          $(".Item, .draggable").each(function(){
            var id = "      #" + $(this).attr("id");
            new_css += id + " {";
            new_css += "left:" + $(this).position().left + "px; ";
            new_css += "top:" + $(this).position().top + "px; ";
            new_css += "}\n";
          });

          $(".Item, .resizable").each(function(){
            var id = "      #" + $(this).attr("id");
            new_css += id + " {";
            new_css += "width:" + $(this).width() + "px; ";
            new_css += "height:" + $(this).height() + "px; ";
            new_css += "}\n";
          });

          $(".Item, .zindexable").each(function(){
            var id = "      #" + $(this).attr("id");
            new_css += id + " {";
            new_css += "z-index:" + $(this).zIndex() + "; ";
            new_css += "}\n";
          });

          $("#css").text(new_css);
        }
      }
    });
  });
]]></script>
