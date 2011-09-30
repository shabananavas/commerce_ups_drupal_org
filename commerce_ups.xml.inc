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


/**
 * Constructs an XML quote request.
 *
 * @param $packages
 *   Array of packages received from the cart.
 * @param $origin
 *   Delivery origin address information.
 * @param $destination
 *   Delivery destination address information.
 * @param $ups_service
 *   UPS service code (refers to UPS Ground, Next-Day Air, etc.).
 *
 * @return
 *   RatingServiceSelectionRequest XML document to send to UPS.
 */
function _commerce_ups_shipping_quote($ups_service, $packages, $origin, $destination) {
  $store['name'] = variable_get('uc_store_name', NULL);
  $store['owner'] = variable_get('uc_store_owner', NULL);
  $store['email'] = variable_get('uc_store_email', NULL);
  $store['email_from'] = variable_get('uc_store_email', NULL);
  $store['phone'] = variable_get('uc_store_phone', NULL);
  $store['fax'] = variable_get('uc_store_fax', NULL);
  $store['street1'] = variable_get('uc_store_street1', NULL);
  $store['street2'] = variable_get('uc_store_street2', NULL);
  $store['city'] = variable_get('uc_store_city', NULL);
  $store['zone'] = variable_get('uc_store_zone', NULL);
  $store['postal_code'] = variable_get('uc_store_postal_code', NULL);
  $store['country'] = variable_get('uc_store_country', 840);

  $account = variable_get('uc_ups_shipper_number', '');
  $ua = explode(' ', $_SERVER['HTTP_USER_AGENT']);
  $user_agent = $ua[0];

  $services = _commerce_ups_service_list();
  $service = array('code' => $ups_service, 'description' => $services[$ups_service]['description']);

  $pkg_types = _commerce_ups_pkg_types();

  $shipper_zone = uc_get_zone_code($store['zone']);
  $shipper_country = uc_get_country_data(array('country_id' => $store['country']));
  $shipper_country = $shipper_country[0]['country_iso_code_2'];
  $shipper_zip = $store['postal_code'];
  $shipto_zone = uc_get_zone_code($destination->zone);
  $shipto_country = uc_get_country_data(array('country_id' => $destination->country));
  $shipto_country = $shipto_country[0]['country_iso_code_2'];
  $shipto_zip = $destination->postal_code;
  $shipfrom_zone = uc_get_zone_code($origin->zone);
  $shipfrom_country = uc_get_country_data(array('country_id' => $origin->country));
  $shipfrom_country = $shipfrom_country[0]['country_iso_code_2'];
  $shipfrom_zip = $origin->postal_code;

  $ups_units = variable_get('uc_ups_unit_system', variable_get('uc_length_unit', 'in'));
  switch ($ups_units) {
    case 'in':
      $units = 'LBS';
      $unit_name = 'Pounds';
      break;
    case 'cm':
      $units = 'KGS';
      $unit_name = 'Kilograms';
      break;
  }

  $shipment_weight = 0;
  $package_schema = '';
  foreach ($packages as $package) {

    // Determine length conversion factor and weight conversion factor
    // for this shipment
    $length_factor = uc_length_conversion($package->length_units, variable_get('uc_ups_unit_system', variable_get('uc_length_unit', 'in')));
    switch ($ups_units) {
      case 'in':
        $weight_factor = uc_weight_conversion($package->weight_units, 'lb');
        break;
      case 'cm':
        $weight_factor = uc_weight_conversion($package->weight_units, 'kg');
        break;
    }

    // Loop over quantity of packages in this shipment
    $qty = $package->qty;
    for ($i = 0; $i < $qty; $i++) {
      // Build XML for this package
      $package_type = array('code' => $package->pkg_type, 'description' => $pkg_types[$package->pkg_type]);
      $package_schema .= "<Package>";
      $package_schema .=   "<PackagingType>";
      $package_schema .=     "<Code>" . $package_type['code'] . "</Code>";
      $package_schema .=   "</PackagingType>";
      if ($package->pkg_type == '02' && $package->length && $package->width && $package->height) {
        if ($package->length < $package->width) {
          list($package->length, $package->width) = array($package->width, $package->length);
        }
        $package_schema .= "<Dimensions>";
        $package_schema .=   "<UnitOfMeasurement>";
        $package_schema .=     "<Code>" . strtoupper(variable_get('uc_ups_unit_system', variable_get('uc_length_unit', 'in'))) . "</Code>";
        $package_schema .=   "</UnitOfMeasurement>";
        $package_schema .=   "<Length>" . number_format($package->length * $length_factor, 2, '.', '') . "</Length>";
        $package_schema .=   "<Width>" . number_format($package->width * $length_factor, 2, '.', '') . "</Width>";
        $package_schema .=   "<Height>" . number_format($package->height * $length_factor, 2, '.', '') . "</Height>";
        $package_schema .= "</Dimensions>";
      }

      $weight = max(1, $package->weight * $weight_factor);
      $shipment_weight += $weight;
      $package_schema .=   "<PackageWeight>";
      $package_schema .=     "<UnitOfMeasurement>";
      $package_schema .=       "<Code>" . $units . "</Code>";
      $package_schema .=       "<Description>" . $unit_name . "</Description>";
      $package_schema .=     "</UnitOfMeasurement>";
      $package_schema .=     "<Weight>" . number_format($weight, 1, '.', '') . "</Weight>";
      $package_schema .=   "</PackageWeight>";

      $size = $package->length * $length_factor + 2 * $length_factor * ($package->width + $package->height);
      if ($size > 130 && $size <= 165) {
        $package_schema .= "<LargePackageIndicator/>";
      }

      if (variable_get('uc_ups_insurance', TRUE)) {
        $package_schema .= "<PackageServiceOptions>";
        $package_schema .=   "<InsuredValue>";
        $package_schema .=     "<CurrencyCode>" . variable_get('uc_currency_code', 'USD') . "</CurrencyCode>";
        $package_schema .=     "<MonetaryValue>" . $package->price . "</MonetaryValue>";
        $package_schema .=   "</InsuredValue>";
        $package_schema .= "</PackageServiceOptions>";
      }
      $package_schema .= "</Package>";
    }
  }

  $schema = _commerce_ups_xml_access_request() . "
<?xml version=\"1.0\" encoding=\"UTF-8\"?>
<RatingServiceSelectionRequest xml:lang=\"en-US\">
  <Request>
    <TransactionReference>
      <CustomerContext>Complex Rate Request</CustomerContext>
      <XpciVersion>1.0001</XpciVersion>
    </TransactionReference>
    <RequestAction>Rate</RequestAction>
    <RequestOption>rate</RequestOption>
  </Request>
  <PickupType>
    <Code>" . variable_get('uc_ups_pickup_type', '01') . "</Code>
  </PickupType>
  <CustomerClassification>
    <Code>" . variable_get('uc_ups_classification', '04') . "</Code>
  </CustomerClassification>
  <Shipment>
    <Shipper>
      <ShipperNumber>" . variable_get('uc_ups_shipper_number', '') . "</ShipperNumber>
      <Address>
        <City>" . $store['city'] . "</City>
        <StateProvinceCode>$shipper_zone</StateProvinceCode>
        <PostalCode>$shipper_zip</PostalCode>
        <CountryCode>$shipper_country</CountryCode>
      </Address>
    </Shipper>
    <ShipTo>
      <Address>
        <StateProvinceCode>$shipto_zone</StateProvinceCode>
        <PostalCode>$shipto_zip</PostalCode>
        <CountryCode>$shipto_country</CountryCode>
      ";
      if (variable_get('uc_ups_residential_quotes', 0)) {
        $schema .= "<ResidentialAddressIndicator/>
      ";
      }
      $schema .= "</Address>
    </ShipTo>
    <ShipFrom>
      <Address>
        <StateProvinceCode>$shipfrom_zone</StateProvinceCode>
        <PostalCode>$shipfrom_zip</PostalCode>
        <CountryCode>$shipfrom_country</CountryCode>
      </Address>
    </ShipFrom>
    <ShipmentWeight>
      <UnitOfMeasurement>
        <Code>$units</Code>
        <Description>$unit_name</Description>
      </UnitOfMeasurement>
      <Weight>" . number_format($shipment_weight, 1, '.', '') . "</Weight>
    </ShipmentWeight>
    <Service>
      <Code>{$service['code']}</Code>
      <Description>{$service['description']}</Description>
    </Service>
    ";
    $schema .= $package_schema;
    if (variable_get('uc_ups_negotiated_rates', FALSE)) {
      $schema .= "<RateInformation>
          <NegotiatedRatesIndicator/>
        </RateInformation>";
    }
  $schema .= "</Shipment>
</RatingServiceSelectionRequest>";

  return $schema;
}