<?php


/**
 * Returns XML access request to be prepended to all requests to UPS
 */
function _commerce_ups_xml_access_request() {
  $access = variable_get('commerce_ups_access_key', '');
  $user = variable_get('commerce_ups_user_id', '');
  $password = variable_get('commerce_ups_password', '');
  return "<?xml version=\"1.0\" encoding=\"UTF-8\"?>
<AccessRequest xml:lang=\"en-US\">
  <AccessLicenseNumber>$access</AccessLicenseNumber>
  <UserId>$user</UserId>
  <Password>$password</Password>
</AccessRequest>
";
}



function _commerce_ups_get_shipping_rate($shipping_service, $order) {
  
  $aXml = array();
  
  /* Request Type */
  $aXml['Request'] = array(
    "RequestAction" => "Rate",
    "RequestOption" => "Rate"
  );
  
  
  /* Pickup Schedule */
  $arrSchedules = _commerce_ups_pickup_types();
  $scheduleCode = variable_get("commerce_ups_pick_up_schedule");
  $aXml['PickupType'] = array(
    "Code" => $scheduleCode,
    "Description" => $arrSchedules[$scheduleCode]
  );
  
  
  /* Shipment */
  $aXml['Shipment'] = array();
  
  
  /* This is the company location, not necessarily
  where the order is being shipped from. Since we 
  don't differentiate yet with this mod, we'll just
  use the same information */
  $aXml['Shipment']['Shipper'] = array(
    "ShipperNumber" => variable_get("commerce_ups_account_id")
  );
  $aXml['Shipment']['Shipper']['Address'] = array(
    "AddressLine1" => variable_get("commerce_ups_address_line_1"),
    "AddressLine2" => variable_get("commerce_ups_address_line_2"),
    "City" => variable_get("commerce_ups_city"),
    "StateProvinceCode" => variable_get("commerce_ups_state"),
    "PostalCode" => variable_get("commerce_ups_postal_code"),
    "CountryCode" => variable_get("commerce_ups_country_code"),
  );
  
  
  /* Ship To - Customer Shipping Address */
  $order_wrapper = entity_metadata_wrapper('commerce_order', $order);
  // Prepare the shipping address for use in the request.
  if (!empty($order_wrapper->commerce_customer_shipping->commerce_customer_address)) {
    $shipping_address = $order_wrapper->commerce_customer_shipping->commerce_customer_address->value();
  }
  $aXml['Shipment']['ShipTo'] = array(
    "CompanyName" =>  $shipping_address['name_line'],
  );
  $aXml['Shipment']['ShipTo']['Address'] = array(
    "AddressLine1" => $shipping_address['thoroughfare'],
    "AddressLine2" => $shipping_address['premise'],
    "AddressLine3" => $shipping_address['sub_premise'],
    "City" => $shipping_address['locality'],
    "StateProvinceCode" => $shipping_address['administrative_area'],
    "PostalCode" => $shipping_address['postal_code'], 
    "CountryCode" => $shipping_address['country']
  );
  
  
  /* Ship From */
  $aXml['Shipment']['ShipFrom'] = array(
    "CompanyName" => variable_get("commerce_ups_company_name")
  );
  $aXml['Shipment']['ShipFrom']['Address'] = array(
    "AddressLine1" => variable_get("commerce_ups_address_line_1"),
    "AddressLine2" => variable_get("commerce_ups_address_line_2"),
    "City" => variable_get("commerce_ups_city"),
    "StateProvinceCode" => variable_get("commerce_ups_state"),
    "PostalCode" => variable_get("commerce_ups_postal_code"),
    "CountryCode" => variable_get("commerce_ups_country_code"),
  );
  
  /* Look up Service code by provided slug */
  $code = null;
  $arrServices = _commerce_ups_service_list();
  foreach($arrServices as $key => $service) {
    if($service['slug'] == $shipping_service['name'])
      $code = $key;
  }
  $aXml['Shipment']['Service'] = array(
    "Code" => $code 
  );

  
  
  
  
  $xml = new SimpleXMLElement("<?xml version=\"1.0\"?><RatingServiceSelectionRequest></RatingServiceSelectionRequest>");
  array_to_xml($aXml, $xml);
  dpm($aXml);
  // dpm($xml->saveXML());
  
  
  
  
  /* Replace with Payload */
  return array(
    'amount' => 2000,
    'currency_code' => 'USD',
    'data' => array(),
  );

}






function _array_to_xml($student_info, &$xml_student_info) {
  foreach($student_info as $key => $value) {
    if(is_array($value)) {
      if(!is_numeric($key)) {
        $subnode = $xml_student_info->addChild("$key");
        array_to_xml($value, $subnode);
      } else {
        array_to_xml($value, $xml_student_info);
      }
    } else {
      $xml_student_info->addChild("$key","$value");
    }
  }
}
