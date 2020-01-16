require 'bundler/inline'

gemfile do
  source 'https://rubygems.org'
  gem 'nexmo'
end

## map uid's to phone numbers - sensitive data, please secure this file
## to find your uid type 'id -u' in console
$users = { 
	:'0' => '351910000000'
}

$client = Nexmo::Client.new(
  api_key: '073d7b17',
  api_secret: '0aSE2jrehK6KDsh7'
)

class SmsService
  def self.send(uid)

    phone_number = $users[uid.to_sym].nil? ? 'unknown' : $users[uid.to_sym].nil?

    response = $client.verify.request(
      number: phone_number,
            country: 'PT',
            brand: 'FILE AUTH',
            code_length: '6',
            pin_expiry: '120'
    )
    
    log_output = open('/root/fuseAuth/auth.log','a+')
    log_output.puts 'uid: ' + uid + ' attempted sent message to ' + phone_number
    log_output.flush

    if response.status == '0'
      request_id = response.request_id
      puts request_id
      log_output.puts 'uid: ' + uid + ' ' + request_id
      log_output.flush
      log_output.close
      output = open('/root/fuseAuth/get_request_id','w');
      output.puts request_id
      output.flush
      output.close
      exit(0)
    else
      puts response.error_text
      log_output.puts 'uid: ' + uid + ' ' + response.error_text
      log_output.flush
      log_output.close
      output = open('/root/fuseAuth/get_request_id','w');
      output.puts "-1" ## send error signal
      output.flush
      output.close
      exit(1)
    end
  end

  def self.verify(code, request_id)
    response = $client.verify.check(
      request_id: request_id,
      code: code
    )

    output = open('/root/fuseAuth/auth.log','a+');

    if response.status == '0'
      # the cost of this verification
      # puts 'Cost ' + response.price + response.currency
      puts response.status
      output.puts  'SUCCESS Code ' + code + ' : ' + request_id #log
      output.flush
      output.close
      exit(0)
    else
      output.puts response.error_text #log
      output.flush
      output.close
      puts response.error_text
      exit(1)
    end
  end
end

if ARGV[0] == "send"
  SmsService.send(ARGV[1])
else
  SmsService.verify(ARGV[1], ARGV[2])
end

