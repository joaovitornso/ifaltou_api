from django.shortcuts import render
from django.http import JsonResponse
from django.views.decorators.csrf import csrf_exempt
import json
from rest_framework import generics
from .models import NFCReading
from .serializers import NFCSerializer
from drf_yasg.utils import swagger_auto_schema
from drf_yasg import openapi
from django.utils import timezone
from rest_framework.decorators import api_view

class NFCAPIListView(generics.ListCreateAPIView):
    queryset = NFCReading.objects.all()
    serializer_class = NFCSerializer

    @swagger_auto_schema(
        operation_description="Lista todos os NFCAPIs",
        responses={200: NFCSerializer(many=True)},
    )
    def get(self, request, *args, **kwargs):
        return super().get(request, *args, **kwargs)

    @swagger_auto_schema(
        operation_description="Cria um novo NFCAPI",
        request_body=NFCSerializer,
        responses={201: NFCSerializer()},
    )
    def post(self, request, *args, **kwargs):
        return super().post(request, *args, **kwargs)
    
@api_view(['GET', 'POST'])    
@csrf_exempt
def receber_json(request):
    if request.method in ['GET', 'POST']:
        tag = request.GET.get('tag')
        print("TAG: " + str(tag))
        
        if request.method == 'POST': 
            tag = request.GET.get('tag')
            print("-- -- -- -- -- Requisição POST -- -- -- -- --") 
            print("TAG: " + str(tag))
            print(request.body)
            
            try:
                # Obtendo os dados JSON da requisição POST
                data = json.loads(request.body)
                
                 # Criando uma instância do modelo NFCReading com os dados recebidos
                nfc_reading = NFCReading(
                    tag=data.get('tag_value', ''),
                    timestamp = timezone.now()
                )

                # Salvando a instância no banco de dados
                nfc_reading.save()
                print("TAG adicionada ao banco de dados")
                print(nfc_reading)
                
                tag = request.GET.get('tag')
                
                print("Aqui está a TAG: " + str(tag))
                
                tag_value = data.get('tag_value', '')
                print("Aqui está a TAG Value: " + str(tag_value))
                # Aqui podemos processar os dados conforme necessário
                # Como salvar os dados no banco de dados ou realizar outras operações 

                # Enviando uma resposta de sucesso
                response_data = {'status': 'success', 'message': 'Dados recebidos com sucesso', 'tag': tag}
                return JsonResponse(response_data)
            except json.JSONDecodeError as e:
                # Lidar com erros de decodificação JSON, se necessário
                response_data = {'status': 'error', 'message': f'Erro na decodificacao JSON: {str(e)}'}
                return JsonResponse(response_data, status=400)
        
        else:
            # Se for uma requisição GET
            response_data = {'status': 'info', 'message': 'Requisicao GET recebida', 'tag': tag}
            return JsonResponse(response_data)
        
    else:
        # Se a requisição não for POST ou GET, retornar um erro
        response_data = {'status': 'error', 'message': 'Apenas requisicoes POST e GET sao suportadas'}
        return JsonResponse(response_data, status=405)
    
@api_view(['GET', 'POST']) 
def get_buffer(request):
    if request.method == 'POST':
        tags = request.GET.get('tags') 
        print("TAGs: " + str(tags))
    
        print(request.body)
        try:
            # Obtendo os dados JSON da requisição POST
            data = json.loads(request.body)
            tags = request.GET.get('tags')
            
            print("Aqui estão as TAGs: " + str(tags))
            
           
            # Aqui você pode processar os dados conforme necessário
            # Por exemplo, você pode salvar os dados no banco de dados
            # ou realizar outras operações comerciais

            # Enviando uma resposta de sucesso
            response_data = {'status': 'success', 'message': 'Dados do buffer recebidos com sucesso', 'tags': tags}
            return JsonResponse(response_data)
        except json.JSONDecodeError as e:
            # Lidar com erros de decodificação JSON, se necessário
            response_data = {'status': 'error', 'message': f'Erro na decodificacao JSON do buffer: {str(e)}'}
            return JsonResponse(response_data, status=400)
    else:
        # Se a requisição não for POST, retornar um erro
        response_data = {'status': 'error', 'message': 'BUFFER: Apenas requisicoes POST sao suportadas'}
        return JsonResponse(response_data, status=405)   
        
            