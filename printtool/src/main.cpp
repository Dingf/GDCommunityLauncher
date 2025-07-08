#include <iostream>
#include "PrintTool.h"

#include <memory>
#include "Websocket.h"
#include "URI.h"
#include <cpprest/http_client.h>
#include "StringConvert.h"
#include "FileReader.h"
#include "Character.h"



void BuildCharacterInfo(web::json::value& characterInfo, web::json::value& characterJSON)
{
	uint32_t currentDifficulty = characterJSON[U("InfoBlock")][U("CurrentDifficulty")].as_integer() & 0x7F;

	characterInfo[U("name")] = characterJSON[U("HeaderBlock")][U("Name")];
	characterInfo[U("level")] = characterJSON[U("HeaderBlock")][U("Level")];
	characterInfo[U("className")] = characterJSON[U("HeaderBlock")][U("ClassName")];
	characterInfo[U("hardcore")] = characterJSON[U("HeaderBlock")][U("Hardcore")];
	characterInfo[U("maxDifficulty")] = characterJSON[U("InfoBlock")][U("MaxDifficulty")];
	characterInfo[U("currentDifficulty")] = currentDifficulty;
	characterInfo[U("deathCount")] = characterJSON[U("StatsBlock")][U("Deaths")];
	characterInfo[U("timePlayed")] = characterJSON[U("StatsBlock")][U("PlayedTime")];
	characterInfo[U("physique")] = characterJSON[U("AttributesBlock")][U("Physique")];
	characterInfo[U("cunning")] = characterJSON[U("AttributesBlock")][U("Cunning")];
	characterInfo[U("spirit")] = characterJSON[U("AttributesBlock")][U("Spirit")];
	characterInfo[U("devotionPoints")] = characterJSON[U("AttributesBlock")][U("TotalDevotionPoints")];
	characterInfo[U("health")] = characterJSON[U("AttributesBlock")][U("Health")];
	characterInfo[U("energy")] = characterJSON[U("AttributesBlock")][U("Energy")];
	characterInfo[U("lastAttackedBy")] = characterJSON[U("StatsBlock")][U("PerDifficultyStats")][currentDifficulty][U("LastAttackedBy")];
}

class TestHandler
{
	public:
		void operator>>(const std::string& message)
		{
			std::cout << "we are reading " << std::endl;
		}

		void operator<<(const std::string& message)
		{
			std::cout << "We are writing " << std::endl;
		}
	private:
};

int main(int argc, char** argv)
{
	int x = 10;
	std::cin >> x;
	std::cout << "Test1" << std::endl;

	std::string authToken;
	std::string refreshToken;

	URI endpoint = URI("https://gdcl-api.azurewebsites.net") / "Account" / "login";

	web::json::value requestBody;
	requestBody[U("username")] = JSONString("MrMonday");
	requestBody[U("password")] = JSONString("GrimLeague_APITesting1!");

	web::http::http_request request(web::http::methods::POST);
	request.set_body(requestBody);

	web::http::client::http_client httpClient((utility::string_t)endpoint);
	web::http::http_response response = httpClient.request(request).get();
	if (response.status_code() == web::http::status_codes::OK)
	{
		web::json::value responseBody = response.extract_json().get();
		web::json::value authTokenValue = responseBody[U("access_token")];
		web::json::value refreshTokenValue = responseBody[U("refresh_token")];
		if ((!authTokenValue.is_null()) && (!refreshTokenValue.is_null()))
		{
			authToken = JSONString(authTokenValue.serialize());
			refreshToken = JSONString(refreshTokenValue.serialize());
		}
	}
	else
	{
		throw std::runtime_error("Server responded with status code " + std::to_string(response.status_code()));
	}

	Logger::LogMessage(LOG_LEVEL_DEBUG, "DEBUG the authToken is %", authToken);

	// The io_context is required for all I/O
	//asio::io_context ioc;

	// The SSL context is required, and holds certificates
	//ssl::context ctx{ssl::context::tlsv12_client};

	TestHandler asdfa;

	Websocket<TestHandler> test(asdfa);
		
	test.Connect("gdcl-websocket.azurewebsites.net", "443", "/account/connect", authToken);

	//test.Send("{ \"RequestName\": \"GetSeasons\", \"Arguments\": { \"Branch\": \"beta\" } }");
	//test.Send("{ \"RequestName\": \"GetCharacterData\", \"Arguments\": { \"SeasonParticipantId\": 13411, \"CharacterName\": \"Tester7\" } }");
	//test.Send("{ \"RequestName\": \"GetCharacterFile\", \"Arguments\": { \"SeasonParticipantId\": 13411, \"CharacterName\": \"Tester7\" } }");

	FileReader asdf("D:\\SteamLibrary\\steamapps\\common\\Grim Dawn\\player.gdc");
	std::vector<uint8_t> asdsf2(asdf.GetBufferSize(), 0);
	memcpy(&asdsf2[0], asdf.GetBuffer(), asdf.GetBufferSize());

	std::string base64 = BinaryToBase64(asdsf2);

	//std::cout << "the base64 string is " << std::endl << base64 << std::endl;




	Character characterData;
	if (!characterData.ReadFromBuffer((uint8_t*)asdf.GetBuffer(), asdf.GetBufferSize()))
	{
		Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to load character data.");
	}

	bool hardcore = characterData._headerBlock._charIsHardcore;
	web::json::value characterJSON = characterData.ToJSON();
	web::json::value characterInfo = web::json::value::object();

	BuildCharacterInfo(characterInfo, characterJSON);

	web::json::value requestJSON;
	requestJSON[U("characterData")] = web::json::value::object();
	requestJSON[U("characterData")][U("characterInfo")] = characterInfo;
	requestJSON[U("characterData")][U("questInfo")] = web::json::value::object();
	requestJSON[U("seasonParticipantId")] = 13411;
	requestJSON[U("participantCharacterId")] = 0;

	std::string requestaa = "{ \"RequestName\": \"SaveCharacterFile\", \"Arguments\": { \"SeasonParticipantId\": 13411, \"CharacterName\": \"Tester7\", \"NewCharacter\": false }, \"Data\": " + (std::string)JSONString(requestJSON.serialize()) + ", \"File\": \"" + base64 + "\" }";

	test.Send(requestaa);

	std::cin >> x;





	/*asio::io_context ioc;
	ssl::context ctx{ssl::context::tlsv12_client};

	std::cout << "Test2" << std::endl;
	tcp::resolver resolver{ioc};
	websocket::stream<ssl::stream<tcp::socket>> ws{ioc, ctx};

	std::string host = "gdcl-websocket.azurewebsites.net";
	std::string port = "443";

	std::cout << "Test3" << std::endl;
	// Look up the domain name
	auto const results = resolver.resolve(host, port);

	std::cout << "Test4" << std::endl;
	// Make the connection on the IP address we get from a lookup
	auto ep = asio::connect(beast::get_lowest_layer(ws), results);

	std::cout << "Test5" << std::endl;
	// Set SNI Hostname (many hosts need this to handshake successfully)
	if (!SSL_set_tlsext_host_name(ws.next_layer().native_handle(), host.c_str()))
	{
		std::cout << "wtf bro" << std::endl;
		return 0;
	}
	std::cout << "Test6" << std::endl;

	host += ':' + std::to_string(ep.port());

	// Perform the SSL handshake
	ws.next_layer().handshake(ssl::stream_base::client);

	std::cout << "Test7" << std::endl;
	// Set a decorator to change the User-Agent of the handshake
	ws.set_option(websocket::stream_base::decorator(
		[](websocket::request_type& req)
		{
			req.set(beast::http::field::user_agent,
				std::string(BOOST_BEAST_VERSION_STRING) +
				" websocket-client-coro");
		}));

	std::cout << "Test8" << std::endl;
	// Perform the websocket handshake
	ws.handshake(host, "/season/latest?branch=prod");

	std::cout << "Test9" << std::endl;

	beast::flat_buffer buffer;

	// Read a message into our buffer
	ws.read(buffer);

	std::cout << "Test10" << std::endl;
	std::cout << beast::make_printable(buffer.data()) << std::endl;
	// Close the WebSocket connection
	try
	{

		ws.close(websocket::close_code::normal);
	}
	catch (std::exception& ex)
	{
		std::cout << "yo wtf " << ex.what() << std::endl;
	}

	std::cout << "Test11" << std::endl;*/
	// If we get here then the connection is closed gracefully















	// The make_printable() function helps print a ConstBufferSequence
	/*if (argc == 2)
	{
		PrintTool::Run(argv[1]);
	}
	else
	{
		std::cout << "PRINTTOOL [in]\n\n";
		std::cout << "\t[in] - Specifies a directory or file to print. If a directory is specified, all valid files within the directory will be printed.\n\n";
	}*/
	return 0;
}