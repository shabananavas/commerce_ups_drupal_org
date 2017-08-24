<?php

namespace Drupal\Tests\commerce_ups\Unit;

use Drupal\commerce_ups\UPSRateRequest;

/**
 * Class UPSRateRequestTest.
 *
 * @package Drupal\Tests\commerce_ups\Unit
 */
class UPSRateRequestTest extends UPSUnitTestBase {
  protected $rate_request;

  /**
   * Set up requirements for test.
   */
  public function setUp() {
    parent::setUp();
    $this->rate_request = new UPSRateRequest();
    $this->rate_request->setConfig($this->configuration);
  }

  /**
   * Test getAuth response.
   */
  public function testAuth() {
    $auth = $this->rate_request->getAuth();
    $this->assertEquals($auth['access_key'], '123');
    $this->assertEquals($auth['user_id'], '123');
    $this->assertEquals($auth['password'], '123');
  }

  /**
   * Test useIntegrationMode().
   */
  public function testIntegrationMode() {
    $mode = $this->rate_request->useIntegrationMode();
    $this->assertEquals(TRUE, $mode);
  }

  /**
   * Test getRateType().
   */
  public function testRateType() {
    $type = $this->rate_request->getRateType();
    $this->assertEquals(TRUE, $type);
  }

}
