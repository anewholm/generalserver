<xsl:stylesheet xmlns="http://www.w3.org/1999/xhtml" xmlns:flow="http://exslt.org/flow" xmlns:gs="http://general_server.org/xmlnamespaces/general_server/2006" xmlns:str="http://exslt.org/strings" xmlns:response="http://general_server.org/xmlnamespaces/response/2006" xmlns:database="http://general_server.org/xmlnamespaces/database/2006" xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" response:server-side-only="true" name="code_render" version="1.0" extension-element-prefixes="flow database str debug request regexp repository">
  <!-- gs_code_render - recursive, overridable node output
    all part of the @mode process:
      gs_HTTP_render => gs_code_render
      with gs_inheritance_render
    does not output
      @repository:*
      *[@response:server-side-only]
      xsl:<top level>/@xml:id
  -->
  
  <!-- code rendering -->
  <xsl:include xpath="~XSL/code_render"/>
  <xsl:include xpath="~XSLStylesheet/code_render"/>
  <xsl:include xpath="~XSLTemplate/code_render"/>
  <xsl:include xpath="~XSLInclude/code_render"/>
  <!-- direct inheritance rendering: we call directly to the XSLTemplate bypassing the XSLStylesheet -->
  <!-- xsl:include xpath="~XSLStylesheet/inheritance_render"/ -->
  <xsl:include xpath="~XSLTemplate/inheritance_render"/>
</xsl:stylesheet>