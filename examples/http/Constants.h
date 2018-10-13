/*

MIT License

Copyright (c) 2018 Chris McArthur, prince.chrismc(at)gmail(dot)com

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

#pragma once

enum HttpVersion
{
   HttpVersionInvalid = 0,
   HttpVersion10,
   HttpVersion11,
   HttpVersionLast
};

enum HttpStatus
{
   HttpStatusInvalid,
   HttpStatusOk = 200,
   HttpStatusCreated = 201,
   HttpStatusAccepted = 202,
   HttpStatusNoContent = 204,
   HttpStatusBadRequest = 400,
   HttpStatusNotFound = 404,
   HttpStatusMethodNotAllowed = 405,
   HttpStatusPreConditionFailed = 412,
   HttpStatusRequestEntityTooLarge = 413,
   HttpStatusUnsupportedMediaType = 415,
   HttpStatusRequestedRangeNotSatisfiable = 416,
   HttpStatusInternalServerError = 500,
   HttpStatusNotImplemented = 501,
   HttpStatusHttpVersionNotSupported = 505
};

enum HttpContentType
{
   HttpContentText,
   HttpContentHtml,
   HttpContentJson,
   HttpContentInvalid
};

enum HttpRequestMethod
{
   HttpRequestInvalid = 0,
   HttpRequestOptions,
   HttpRequestGet,
   HttpRequestHead,
   HttpRequestPost,
   HttpRequestPut,
   HttpRequestDelete,
   HttpRequestTrace,
   HttpRequestConnect,
   HttpRequestPatch,
   HttpRequestLast
};
