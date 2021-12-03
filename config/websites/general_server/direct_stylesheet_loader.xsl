<xsl:stylesheet xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns:database="http://general_server.org/xmlnamespaces/database/2006" xmlns="http://www.w3.org/1999/xhtml" xmlns:html="http://www.w3.org/1999/xhtml" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:gs="http://general_server.org/xmlnamespaces/general_server/2006" name="direct_stylesheet_loader" version="1.0">
  <!-- load from stylesheet directly
    normally an object:Response is accessed and response-loader.xsl is used
    however, direct access to an xsl:stylesheet is demonstrated here
    
    object:Response and meta:environment required for the Linked Server and external jQuery
    otherwise external libraries will be loaded from this same server
  -->
  <!-- DXSL -->
  <xsl:include href="/~Class/interface_translators.dxsl"/>
  <xsl:include href="/~Class/class_introspection.dxsl"/>
  <xsl:include href="/~Class/website_classes_html_view_templates.dxsl"/>
  <xsl:include href="/~Class/website_classes_html_dependencies.dxsl"/>

  <interface:HTMLWebpage title="{str:dynamic(descendant::h1[1])}" jQuery="1.10.2" general-server-framework="1.0" code-class="include" code-class-dependencies="include" class-styles="include" class-style-dependencies="include">
    <interface:HTMLContainer>
      <h1>direct stylesheet loader</h1>
      <p>normally an object:Response is accessed and response-loader.xsl is used
      however, direct access to an xsl:stylesheet is demonstrated here</p>
    </interface:HTMLContainer>
  </interface:HTMLWebpage>
</xsl:stylesheet>