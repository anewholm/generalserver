<xsl:stylesheet xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns="http://www.w3.org/1999/xhtml" name="website_not_found" version="1.0" extension-element-prefixes="request">
  <xsl:template match="object:Response">
    <html>
      <head/>

      <body>
        <div class="gs-central-screen">website not found</div>
      </body>
    </html>
  </xsl:template>
</xsl:stylesheet>