import requests
from requests.auth import HTTPBasicAuth

def request_handler(request):
    message = request['form']['text']
    r = requests.post('https://api.twilio.com/2010-04-01/Accounts/AC3e032d8f15c87c6fae7b7a92528be27f/Messages.json',auth=HTTPBasicAuth('AC3e032d8f15c87c6fae7b7a92528be27f', 'cc1aa85f5b554b2e15ea0585e0375f1c'), data = {'To':'+18144943971','MessagingServiceSid':'MG2dba0ee552919a30d9bb6cf575de853e','Body':f'{message}'})
    return(r)