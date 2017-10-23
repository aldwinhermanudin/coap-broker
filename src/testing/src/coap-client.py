import logging
import asyncio

from aiocoap import *

logging.basicConfig(level=logging.INFO)

async def main():
	counter = 0
    
	while(counter < 20):
		context = await Context.create_client_context()

		await asyncio.sleep(1)

		payload = b"54%"
		request = Message(code=PUT, payload=payload)
		request.opt.uri_host = 'fe80::a720:4523:e847:9e52%wlp2s0'
		request.opt.uri_path = ("ps","room1","humid")
		request.opt.content_format = 0
		max_age = optiontypes.UintOption(14, value=counter)
		request.opt.add_option(max_age)

		response = await context.request(request).response

		print('Result: %s\n%r'%(response.code, response.payload))
		counter+=1
	
	#print ('finished\n')

if __name__ == "__main__":
    asyncio.get_event_loop().run_until_complete(main())
