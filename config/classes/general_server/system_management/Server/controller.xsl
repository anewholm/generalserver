<xsl:stylesheet xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns:conversation="http://general_server.org/xmlnamespaces/conversation/2006" xmlns:gs="http://general_server.org/xmlnamespaces/general_server/2006" xmlns:exsl="http://exslt.org/common" xmlns:class="http://general_server.org/xmlnamespaces/class/2006" xmlns:user="http://general_server.org/xmlnamespaces/user/2006" xmlns:debug="http://general_server.org/xmlnamespaces/debug/2006" xmlns:database="http://general_server.org/xmlnamespaces/database/2006" response:server-side-only="true" name="controller" controller="true" version="1.0" extension-element-prefixes="dyn str regexp database conversation server request debug exsl">
  <!-- WATCH an area:
    long-polling version: client should re-request immediately after each server event
    NOT_CURRENTLY_USED:   looped continuous server side transform for incremental javascript snippets
    database:wait-for-single-write-event will hold each transform until the self-or-decendent::target changes
    html Content-type is *necessary* for the slow feed of the script
  -->
  <!-- xsl:output method="html" version="4.01" encoding="utf-8" doctype-public="none"/ -->

  <!-- ##############################################################################
       step 1: retrieve pointers to the changed node(s)
       ############################################################################## -->
  <xsl:template match="object:Server" mode="listen-server-events-start">
    <xsl:variable name="gs_form_xpath_to_node" select="($gs_form/gs:xpath-to-node/* | .)[1]"/>
    <xsl:variable name="gs_form_last_known_event_id" select="$gs_form/gs:last-known-event-id"/>
    <database:wait-for-single-write-event select="$gs_form_xpath_to_node" transform="$gs_stylesheet_server_side_classes" interface-mode="gs_changed_node" last-known-event-id="{$gs_form_last_known_event_id}"/>
  </xsl:template>

  <!-- ##############################################################################
       step 2: (server side) data transformation to include the container object element and ancestors
       ############################################################################## -->
  <xsl:template match="*" mode="gs_changed_node">
    <!-- 
      gs:primary-path is REQUIRED
      FIRST element is the primary node on which to trigger any events
      EACH element has ONLY attribute details of each including:
        @xml:id 
        @meta:xpath-to-node
    -->
    <xsl:param name="gs_event_id"/>
    
    <gs:server-event event-id="{$gs_event_id}">
      <gs:node-changed>
        <gs:primary-path>
          <xsl:apply-templates select="ancestor-or-self::*" mode="gs_ancestor_path"/>
        </gs:primary-path>
      </gs:node-changed>
    </gs:server-event>
  </xsl:template>

  <xsl:template match="*" mode="gs_ancestor_path">
    <xsl:copy>
      <xsl:apply-templates select="." mode="gs_interface_render_adorn_node"/>
      <xsl:apply-templates select="@*" mode="gs_interface_render"/>
    </xsl:copy>
  </xsl:template>
</xsl:stylesheet>
