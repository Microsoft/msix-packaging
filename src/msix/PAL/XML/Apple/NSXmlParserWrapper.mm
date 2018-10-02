//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#import "NSXmlParserWrapper.h"
#import "NSXmlParserDelegateWrapper.h"

namespace MSIX
{
    bool NSXmlParserWrapper::Parse(uint8_t* data, size_t dataLength, void* xmlDocumentReader)
    {
        NSData *xmldata = [NSData dataWithBytes:data length:dataLength];
        wrapped = [[NSXMLParser alloc] initWithData:xmldata];
        
        // Create an instance of our parser delegate and assign it to the parser
        NSXmlParserDelegateWrapper *parserDelegate = [[NSXmlParserDelegateWrapper alloc] initWithXmlDocumentReader:xmlDocumentReader];
        [(NSXMLParser*)wrapped setDelegate:parserDelegate];

        return [(NSXMLParser*)wrapped parse];
    }
}
