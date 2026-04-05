<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns:gs="http://general_server.org/xmlnamespaces/general_server/2006" xmlns:interface="http://general_server.org/xmlnamespaces/interface/2006" xmlns:object="http://general_server.org/xmlnamespaces/object/2006" xmlns="http://www.w3.org/1999/xhtml" name="page_not_found" version="1.0">
  <object:Response stage="{$gs_stage}" xml:id-policy-area="ignore">
    <gs:data>
      <interface:HTMLWebpage title="page not found">
        <interface:HTMLContainer>
          <h1>page not found</h1>
        </interface:HTMLContainer>
      </interface:HTMLWebpage>
    </gs:data>
  </object:Response>
</xsl:stylesheet>
