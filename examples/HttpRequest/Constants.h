enum HttpVersion
{
   VersionInvalid = 0,
   Version10,
   Version11,
   VersionLast
};

enum HttpStatus
{
   HttpStatusInvalid,
   HttpStatusOk = 200,
   HttpStatusCreated = 201,
   HttpStatusAccepted = 202,
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
   ContentText,
   ContentHtml,
   ContentJson,
   ContentInvalid
};