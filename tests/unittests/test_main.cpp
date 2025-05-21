#include "test_main.h"
#include "gtest/gtest.h"

int main(int argc, char* argv[]) {
    testing::InitGoogleTest(&argc, argv);
    // Don't run the following tests
    // testing::GTEST_FLAG(filter) = "-*ResponseWriterTest*";
    // testing::GTEST_FLAG(filter) = "-*TestHttpParser*";
    // testing::GTEST_FLAG(filter) = "-*ListenerTest*:*ServerTest*:*TestHttpParser*";
    // testing::GTEST_FLAG(filter) = "-*ListenerTest*:*ServerTest*";
    // testing::GTEST_FLAG(filter) = "-*ListenerTestWithMockLogging*:*ServerWithMockLoggerParametrizedPortTest*";
    // testing::GTEST_FLAG(filter) = "-*ServerTest*";
    // testing::GTEST_FLAG(filter) = "-*ConnHdlrTestOneConnection.TestPersistenceSendInBatches*";
    // testing::GTEST_FLAG(filter) = "-*sendMsgsAsync/ConnectionHdlrTestOneConnection.TestPersistenceSendInBatches*";
    // testing::GTEST_FLAG(filter) = "-*ConnectionTestResend*";
    // testing::GTEST_FLAG(filter) =
    //     "-*pingTestInBatches*:*ConnHdlrTestWithParamInt.multipleRequestsOneConnectionInBatches*";
    // testing::GTEST_FLAG(filter) = "-*ConnHdlrTestMultipleRouter*";

    // only run the following test
    // testing::GTEST_FLAG(filter) = "*ConnHdlrTestMultipleRouter*";
    // testing::GTEST_FLAG(filter) = "*ConnectionTestChunkedSend*";
    testing::GTEST_FLAG(filter) = "*BodyParserTest*";
    // testing::GTEST_FLAG(filter) = "*UploadHdlrErrorTest*";
    // testing::GTEST_FLAG(filter) = "*UploadHdlrTest.chunked*";
    // testing::GTEST_FLAG(filter) = "*ConnHdlrTestMultipleRouter*";
    // testing::GTEST_FLAG(filter) = "*IONotifierTest*";
    // testing::GTEST_FLAG(filter) = "*ConnHdlrTestWithOneConnection.TestPersistenceSendInBatches*";
    // testing::GTEST_FLAG(filter) = "*ConnHdlrTestAsyncMultipleConnections*";
    // testing::GTEST_FLAG(filter) = "*TestHttpParser*";
    // testing::GTEST_FLAG(filter) = "*ConnHdlrTestWithOneConnectionMockLogger*";
    // testing::GTEST_FLAG(filter) = "*ConnHdlrTestAsyncMultipleConnections.sendMsgsAsync*";
    // testing::GTEST_FLAG(filter) = "*ConnHdlrTestWithOneConnectionPerRequest.sendMsgsAsync*";
    // testing::GTEST_FLAG(filter) = "*ConnHdlrTestTestRouting*";
    // testing::GTEST_FLAG(filter) = "*ConnHdlrTestStubUploadHdlr*";
    // testing::GTEST_FLAG(filter) = "*ConnHdlrTestUpload*";
    // testing::GTEST_FLAG(filter) = "*CgiHandlerTest*";
    // testing::GTEST_FLAG(filter) = "*HttpParserTest*";
    // testing::GTEST_FLAG(filter) = "*DeleteHandler*";
    // testing::GTEST_FLAG(filter) = "*UploadHdlr*";
    // testing::GTEST_FLAG(filter) = "*UploadHdlrFileErrorsTest*";
    // testing::GTEST_FLAG(filter) = "*ConnHdlrTestUpload*";
    // testing::GTEST_FLAG(filter) = "*ConnectionTestResend*";
    // testing::GTEST_FLAG(filter) = "*noContentLengthSetsBodyToFinished*";
    // testing::GTEST_FLAG(filter) = "*ConnHdlrTestWithOneConnectionPerRequest.sendMsgsAsync*";
    // testing::GTEST_FLAG(filter) = "*ConnHdlrTestWithOneConnection.TestPersistenceSendInBatches*";
    // testing::GTEST_FLAG(filter) = "*ConnHdlrTestWithOneConnection.TestBadRequestClosesConnection*";
    // testing::GTEST_FLAG(filter) = "*ConnectionHdlrTestWithParamInt.multipleRequestsOneConnectionInBatches*";
    // testing::GTEST_FLAG(filter) = "*ResponseWriterTest*";
    // testing::GTEST_FLAG(filter) = "*testingBatchSizesSending*";
    // testing::GTEST_FLAG(filter) = "*pingTestInBatches*";
    // testing::GTEST_FLAG(filter) = "*multipleRequestsOneConnectionInBatches*";
    // testing::GTEST_FLAG(filter) = "*RouterTest.testWithConfigParsing*";
    // testing::GTEST_FLAG(filter) = "*RouterTest*";
    // testing::GTEST_FLAG(filter) = "*IONotifierTest*";
    // testing::GTEST_FLAG(filter) = "*ConnHdlrTest.send2MsgsAsync*";
    // testing::GTEST_FLAG(filter) = "*ConnHdlrTestWithParamInt.multipleRequestsOneConnectionInBatches*";
    // testing::GTEST_FLAG(filter) = "*ConnHdlrTestOneConnection.TestBadRequestClosesConnection*";
    // testing::GTEST_FLAG(filter) = "*ConnHdlrTestOneConnection.TestPersistenceSendInBatches*";
    // testing::GTEST_FLAG(filter) = "*ConnHdlrTestWithOneConnection.TestPersistenceSendInOneMsg*";
    // testing::GTEST_FLAG(filter) = "*ConnHdlrTestWithParamInt.multipleRequestsOneConnectionInBatches*";
    // testing::GTEST_FLAG(filter) = "*ConnHdlrTest*";
    // testing::GTEST_FLAG(filter) = "*Listener*";
    // testing::GTEST_FLAG(filter) = "*ListenerNotificationTest*";
    // testing::GTEST_FLAG(filter) = "*ListenerTestWithMockLogging*";
    // testing::GTEST_FLAG(filter) = "*ListenerTestWithMockLogging.closing*";
    // testing::GTEST_FLAG(filter) = "*ListenerTestWo*";
    // testing::GTEST_FLAG(filter) = "*ListenerTestSimple.connHdlrGetsCalled*";
    // testing::GTEST_FLAG(filter) = "ListenerTest.closingAConnection";
    // testing::GTEST_FLAG(filter) = "ListenerTest.closingAConnection:ListenerTest.multiplePortsTestWoLogging";
    // testing::GTEST_FLAG(filter) = "ServerWithMockLoggerParametrizedPortTest";
    // testing::GTEST_FLAG(filter) = "ServerTest*";
    // testing::GTEST_FLAG(filter) = "*HttpParser*";
    // testing::GTEST_FLAG(filter) = "*HandlerTest*";
    // testing::GTEST_FLAG(filter) = "*ServerConfig*";
    return RUN_ALL_TESTS();
}
