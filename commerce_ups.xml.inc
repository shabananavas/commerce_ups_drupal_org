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

/*
 * This builds the XML to submit to UPS for rates.
 */
function commerce_ups_build_rate_request($shipping_service, $order) {
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

  // Build and populate the rate request SimpleXML element.
  $rate_request_element = new SimpleXMLElement('<RatingServiceSelectionRequest/>');
  commerce_simplexml_add_children($rate_request_element, $aXml);
  
  dpm($aXml);
  // dpm($xml->saveXML());
  
  return $rate_request_element;

}

/**
 * Submits an API request to the Progistics XML Processor.
 *
 * @param $xml
 *   An XML string to submit to the Progistics XML Processor.
 * @param $message
 *   Optional log message to use for logged API requests.
 */
function commerce_ups_api_request($xml, $message = '') {
  // Log the API request if specified.
  if (in_array('request', variable_get('commerce_ups_log', array()))) {
    if (empty($message)) {
      $message = t('Submitting API request to the UPS');
    }

    watchdog('ups', '@message:<pre>@xml</pre>', array('@message' => $message, '@xml' => $xml));
  }

  // Build the array of header information for the request.
  /*
  $header = array();
  $header[] = 'Content-type: text/xml; charset=utf-8';
  $header[] = 'Content-length: ' . strlen($xml);

  // POST the XML to the XML Processor.
  $ch = curl_init(commerce_connectship_xml_processor_url());
  curl_setopt($ch, CURLOPT_VERBOSE, 0);
  curl_setopt($ch, CURLOPT_POST, 1);
  curl_setopt($ch, CURLOPT_POSTFIELDS, $xml);
  curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
  curl_setopt($ch, CURLOPT_NOPROGRESS, 1);
  curl_setopt($ch, CURLOPT_FOLLOWLOCATION, 0);
  $result = curl_exec($ch);
  */

  // Log any errors to the watchdog.
  if ($error = curl_error($ch)) {
    watchdog('ups', 'cURL error: @error', array('@error' => $error), WATCHDOG_ERROR);
    return FALSE;
  }
  curl_close($ch);

  // If we received data back from the server...
  if (!empty($result)) {
    // Extract the result into an XML response object.
    $response = new SimpleXMLElement($result);

    // Log the API request if specified.
    if (in_array('response', variable_get('commerce_ups_log', array()))) {
      watchdog('ups', 'API response received:<pre>@xml</pre>', array('@xml' => $response->asXML()));
    }

    return $response;
  }
  else {
    return FALSE;
  }
}

function _array_to_xml($student_info, &$xml_student_info) {
  foreach($student_info as $key => $value) {
    if(is_array($value)) {
      if(!is_numeric($key)) {
        $subnode = $xml_student_info->addChild("$key");
        _array_to_xml($value, $subnode);
      } else {
        _array_to_xml($value, $xml_student_info);
      }
    } else {
      $xml_student_info->addChild("$key","$value");
    }
  }
}
