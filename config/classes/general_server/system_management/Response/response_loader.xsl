<xsl:stylesheet xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns:database="http://general_server.org/xmlnamespaces/database/2006" xmlns="http://www.w3.org/1999/xhtml" xmlns:html="http://www.w3.org/1999/xhtml" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:gs="http://general_server.org/xmlnamespaces/general_server/2006" name="response_loader" version="1.0">
  <!--
    interface-loader.xsl is being used soely for its interface:dynamic like the API
    EXCEPT server-side XSLT: interface-loader.xsl is a standard GET request and therefore can be transformed on the server
    normally this stylesheet interface-loader.xsl will NOT BE served with the data because AJAX uses it's parent stylesheet, not this one
      i.e. it calls interface-loader.xsl?explicit-stylesheet=<stylesheet to use>
      
    $gs_query_string/@classes = the list of classes required

    the usual plugins will affect gs_interface_render output
      ~AJAXHTMLLoader/interface_render
      ...

    note the DSXL includes. these are dynamically generated stylesheets
    they will also include the HTTP basics:
      HTTP/html_default_templates.xsl
      HTTP/client_environment_variables.xsl

    (1) the interface:dynamic @... statement will now recursively render the output data ($gs_request_target)
      $gs_request_target => an object:Response
    (2) followed by serving of this stylesheet with its xsl:includes and DXSL expanded
  -->
  <!-- DXSL -->
  <xsl:include href="/~Class/interface_translators.dxsl"/>
  <xsl:include href="/~Class/class_introspection.dxsl"/>
  <xsl:include href="/~Class/website_classes_html_view_templates.dxsl"/>
  <xsl:include href="/~Class/website_classes_html_dependencies.dxsl"/>

  <!-- useful includes for your custom stylesheets -->
  <!-- TODO: xsl:include href="~HTTP/everything.xsl" / -->

  <!-- $gs_query_string options are ignored here. this is a full Response with interfaces and it makes its own decisions -->
  <database:query source="response_loader/gs_request_target" data="$gs_request_target" />
</xsl:stylesheet>
