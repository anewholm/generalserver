<xsl:stylesheet xmlns:xxx="http://general_server.org/xmlnamespaces/dummyxsl/2006" xmlns="http://www.w3.org/1999/xhtml" xmlns:html="http://www.w3.org/1999/xhtml" xmlns:str="http://exslt.org/strings" xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" name="controller" controller="true" response:server-side-only="true" extension-element-prefixes="debug" version="1.0">
  <xsl:template match="debug:GDB-break" mode="gs_interface_render">
    <debug:GDB-break/>
  </xsl:template>

  <xsl:template match="debug:xslt-set-trace" mode="gs_interface_render">
    <debug:xslt-set-trace/>
  </xsl:template>

  <xsl:template match="debug:xslt-clear-trace" mode="gs_interface_render">
    <debug:xslt-clear-trace/>
  </xsl:template>
</xsl:stylesheet>